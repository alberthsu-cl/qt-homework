#include "pch.h"
#include "MainFrame.h"
#include "QtKitHost.h"
#include "DemoNames.h"

#include <afxdlgs.h>   // CFileDialog
#include <shlwapi.h>   // PathFileExists
#include <string>

namespace {

const int kStatusHeight = 30;

std::string ToUtf8(const CString& s)
{
    if (s.IsEmpty()) return std::string();
    int n = ::WideCharToMultiByte(CP_UTF8, 0, s, -1, nullptr, 0, nullptr, nullptr);
    if (n <= 1) return std::string();
    std::string out(static_cast<size_t>(n - 1), '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, s, -1, &out[0], n, nullptr, nullptr);
    return out;
}

} // namespace

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_DESTROY()
    ON_COMMAND(IDM_FILE_OPEN, &CMainFrame::OnFileOpen)
    ON_COMMAND(IDM_FILE_EXIT, &CMainFrame::OnFileExit)
    ON_MESSAGE(WM_APP_QML_CLICK, &CMainFrame::OnQmlClick)
END_MESSAGE_MAP()

CMainFrame::CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpcs)
{
    if (CFrameWnd::OnCreate(lpcs) == -1)
        return -1;

    // ---- a menu, built in code so the demo needs no .rc -------------------
    CMenu menuBar, menuFile;
    menuBar.CreateMenu();
    menuFile.CreatePopupMenu();
    menuFile.AppendMenu(MF_STRING, IDM_FILE_OPEN, _T("&Open Image...\tCtrl+O"));
    menuFile.AppendMenu(MF_SEPARATOR);
    menuFile.AppendMenu(MF_STRING, IDM_FILE_EXIT, _T("E&xit"));
    menuBar.AppendMenu(MF_POPUP, reinterpret_cast<UINT_PTR>(menuFile.Detach()), _T("&File"));
    SetMenu(&menuBar);
    menuBar.Detach();

    // ---- an ordinary MFC child, so native and QML sit side by side --------
    m_wndStatus.Create(_T("  starting QtKit..."), WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE,
                       CRect(0, 0, 0, 0), this, 0);
    m_wndStatus.SetFont(GetFont());

    // ---- start Qt ---------------------------------------------------------
    // The callback lands on the Qt thread.
    if (!QtKitHost::Inst().Start([this]() { OnQtReady(); }))
    {
        m_wndStatus.SetWindowText(
            _T("  QtKit.dll failed to load - is this .exe in bin_x64\\PowerDirector\\ ?"));
    }

    return 0;
}

// =============================================================================
//  QT THREAD from here down (OnQtReady, AdoptQmlWindow, and every lambda)
// =============================================================================

void CMainFrame::OnQtReady()
{
    if (!QtKitHost::Inst().Context())
        return;

    // The .qml is read straight off disk (no skinQt.rcc here), so the entry is
    // a filesystem path relative to the .exe. The .exe lives in
    // <demo>\bin_x64\ and the QML in <demo>\qml\, hence one level up.
    //
    // Shipping builds take the other branch of that fork: PDR packs its QML
    // into skinQt.rcc and loads ":/qml/..." instead. See QTKIT_ARCHITECTURE.md
    // section 07 - every CQtEntryViewController is constructed with both paths
    // and picks between them on isRccFileReady().
    m_strQmlEntry = QtKitHost::Inst().ExeDir() + "../qml/DemoWindow.qml";

    // Adopt only once the entire tree is built. Hanging this off
    // setBindAction(window) instead looks equivalent and is not: the window's
    // bind action fires while objects further down the tree are still unbound.
    QtKitHost::Inst().LoadQml(m_strQmlEntry, [this]() { AdoptQmlWindow(); });
}

void CMainFrame::AdoptQmlWindow()
{
    IQmlContext* pContext = QtKitHost::Inst().Context();
    if (!pContext)
        return;

    QtKitHost::Log("bound? window=%d caption=%d imagePath=%d zoomIn=%d reset=%d",
                   (int)pContext->isObjectBound(DemoName.window),
                   (int)pContext->isObjectBound(DemoName.caption),
                   (int)pContext->isObjectBound(DemoName.imagePath),
                   (int)pContext->isObjectBound(DemoName.zoomInButton),
                   (int)pContext->isObjectBound(DemoName.resetButton));

    if (!pContext->isObjectBound(DemoName.window))
        return;

    // ---- 1. pull the native handle out of Qt ------------------------------
    HWND hQml = nullptr;
    try
    {
        hQml = static_cast<HWND>(pContext->window(DemoName.window).window());
    }
    catch (const QmlObjectNoBoundError& e)
    {
        QtKitHost::Log("window not bound: %s", e.what());
        return;
    }
    if (!hQml)
        return;

    QtKitHost::Log("got HWND %p", (void*)hQml);
    m_hQmlWnd = hQml;

    // ---- 2. reparent it under the MFC frame -------------------------------
    // This is PDR's CQtEntryViewController::FillInParentWnd() in two lines.
    // After SetParent the QQuickWindow is an ordinary Win32 child: no
    // compositing layer, no shared surface, no Qt widget wrapper.
    // Reparent THROUGH QtKit, not with a raw ::SetParent. IUIWindow::setParent
    // and setPosition go through Qt, so the QML scene actually reflows to the
    // new size. A bare ::SetWindowPos moves the native window and leaves the
    // scene at its declared logical size, and anchored content then renders
    // off the right and bottom edges - which looks exactly like a DPI bug and
    // is not one.
    pContext->window(DemoName.window).setParent(GetSafeHwnd());

    // Strip every top-level decoration. Qt.FramelessWindowHint in the .qml
    // covers most of it; these two lines make sure nothing survives to eat
    // client area or paint a second caption inside our frame.
    ::SetWindowLongPtr(hQml, GWL_STYLE,
        (::GetWindowLongPtr(hQml, GWL_STYLE) | WS_CHILD)
        & ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX));
    ::SetWindowLongPtr(hQml, GWL_EXSTYLE,
        ::GetWindowLongPtr(hQml, GWL_EXSTYLE) & ~(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME));
    ::SetWindowPos(hQml, nullptr, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    QtKitHost::Log("reparented under %p", (void*)GetSafeHwnd());

    // ---- 3. wire QML -> C++ ------------------------------------------------
    // ALWAYS check isObjectBound() first. The accessors do not return null for
    // an unknown name: QtKit logs "The id (x) does not exist" and hands back a
    // reference you then fault on. This is the C++ side of the loose,
    // string-keyed boundary, and it is why PDR's controllers are littered with
    // CQtViewController::IsQmlBound() calls.
    //
    // Each lambda runs ON THE QT THREAD, so all it may do is PostMessage.
    struct { const char* name; QmlClick code; } kButtons[] = {
        { DemoName.zoomInButton,  kClickZoomIn  },
        { DemoName.zoomOutButton, kClickZoomOut },
        { DemoName.rotateButton,  kClickRotate  },
        { DemoName.resetButton,   kClickReset   },
    };

    for (const auto& b : kButtons)
    {
        if (!pContext->isObjectBound(b.name))
        {
            QtKitHost::Log("button %s NOT bound - skipped", b.name);
            continue;
        }
        const QmlClick code = b.code;
        pContext->button(b.name).setClickedAction([this, code]() {
            ::PostMessage(GetSafeHwnd(), WM_APP_QML_CLICK, static_cast<WPARAM>(code), 0);
        });
    }

    m_bQmlReady = true;
    QtKitHost::Log("bindings wired - demo ready");

    // Do NOT size the Qt child from here. We are on the Qt thread, moments
    // after load(), and Qt has not finished realising the window - the
    // SetWindowPos lands but the QML scene keeps its declared 1000x700 logical
    // size, so anything anchored to the window bottom renders off-screen.
    // Hand the job to the MFC thread instead, which is where window layout
    // belongs anyway.
    ::PostMessage(GetSafeHwnd(), WM_APP_QML_CLICK, 0, 0);   // 0 = "ready"
}

// =============================================================================
//  MFC MAIN THREAD from here down
// =============================================================================

LRESULT CMainFrame::OnQmlClick(WPARAM wParam, LPARAM /*lParam*/)
{
    // Arrived via PostMessage from a Qt-thread callback. Safe to touch MFC now.
    LPCTSTR pszWhat = _T("");
    switch (static_cast<QmlClick>(wParam))
    {
    case kClickZoomIn:  pszWhat = _T("Zoom in");   break;
    case kClickZoomOut: pszWhat = _T("Zoom out");  break;
    case kClickRotate:  pszWhat = _T("Rotate 90"); break;
    case kClickReset:   pszWhat = _T("Reset");     break;
    default:
        // "ready" - first chance to lay out the adopted Qt child on the thread
        // that owns the frame.
        LayoutChildren();
        m_wndStatus.SetWindowText(
            _T("  QML is live. Native MFC owns this strip and the menu above."));

        // MfcQmlDemo.exe "C:\path	o\picture.png" opens straight into an
        // image, which saves a trip through the file dialog when you are
        // testing the C++ -> QML direction.
        if (__argc > 1 && __targv && __targv[1])
        {
            CString strCmd(__targv[1]);
            strCmd.Trim();
            strCmd.Trim(_T('"'));
            QtKitHost::Log("cmdline arg: '%S' exists=%d",
                           strCmd.GetString(), (int)::PathFileExists(strCmd));
            if (!strCmd.IsEmpty() && ::PathFileExists(strCmd))
                PushImageToQml(strCmd);
        }
        return 0;
    }

    ++m_nClickCount;

    CString str;
    str.Format(_T("  QML button \"%s\" reached C++  (%d clicks so far)  <- this strip is native MFC"),
               pszWhat, m_nClickCount);
    m_wndStatus.SetWindowText(str);
    return 0;
}

void CMainFrame::OnFileOpen()
{
    CFileDialog dlg(TRUE, nullptr, nullptr,
                    OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
                    _T("Images|*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.webp|All files|*.*||"),
                    this);
    if (dlg.DoModal() != IDOK)
        return;

    PushImageToQml(dlg.GetPathName());
}

void CMainFrame::PushImageToQml(const CString& strPath)
{
    IQmlContext* pContext = QtKitHost::Inst().Context();
    if (!pContext || !m_bQmlReady)
        return;

    // Qt wants forward slashes and a file:/// URL.
    CString strUrl(strPath);
    strUrl.Replace(_T('\\'), _T('/'));
    strUrl = _T("file:///") + strUrl;

    CString strName(strPath);
    const int i = strName.ReverseFind(_T('\\'));
    if (i >= 0) strName = strName.Mid(i + 1);

    const std::string strUrlUtf8  = ToUtf8(strUrl);
    const std::string strCaption  = "C++ pushed:  " + ToUtf8(strName);

    // ---- C++ -> QML --------------------------------------------------------
    // We are on the MFC thread here, so hop to the Qt thread before touching
    // any IUI* proxy. This is the mirror of the PostMessage going the other way.
    pContext->runOnQtThread([pContext, strUrlUtf8, strCaption]() {
        if (pContext->isObjectBound(DemoName.imagePath))
            pContext->label(DemoName.imagePath).text(strUrlUtf8.c_str());
        if (pContext->isObjectBound(DemoName.caption))
            pContext->label(DemoName.caption).text(strCaption.c_str());
        QtKitHost::Log("pushed image: %s", strUrlUtf8.c_str());
    });

    CString strTitle;
    strTitle.Format(_T("MFC + QML demo - %s"), static_cast<LPCTSTR>(strName));
    SetWindowText(strTitle);
}

void CMainFrame::OnFileExit()
{
    PostMessage(WM_CLOSE);
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
    CFrameWnd::OnSize(nType, cx, cy);
    LayoutChildren();
}

void CMainFrame::LayoutChildren()
{
    CRect rc;
    GetClientRect(&rc);
    if (rc.IsRectEmpty())
        return;

    // Geometry for the Qt child goes through QtKit so the scene reflows.
    // We are on the MFC thread, so hop first.
    IQmlContext* pContext = QtKitHost::Inst().Context();
    if (m_bQmlReady && pContext && m_hQmlWnd && ::IsWindow(m_hQmlWnd))
    {
        const int cx = rc.Width();
        const int cy = rc.Height() - kStatusHeight;
        QtKitHost::Log("LayoutChildren -> setPosition(0,0,%d,%d)", cx, cy);
        pContext->runOnQtThread([pContext, cx, cy]() {
            if (pContext->isObjectBound(DemoName.window))
                pContext->window(DemoName.window).setPosition(0, 0, cx, cy);
        });
    }
    if (m_wndStatus.GetSafeHwnd())
    {
        m_wndStatus.SetWindowPos(nullptr, 0, rc.Height() - kStatusHeight,
                                 rc.Width(), kStatusHeight,
                                 SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void CMainFrame::OnDestroy()
{
    // Detach the Qt child before our own HWND goes away, then stop the runloop.
    // PDR does the equivalent in CQtEntryViewController::OnQmlWillUnloadImp.
    if (m_hQmlWnd && ::IsWindow(m_hQmlWnd))
    {
        ::SetParent(m_hQmlWnd, nullptr);
        m_hQmlWnd = nullptr;
    }
    if (!m_strQmlEntry.empty())
        QtKitHost::Inst().UnloadQml(m_strQmlEntry);

    QtKitHost::Inst().Stop();

    CFrameWnd::OnDestroy();
}
