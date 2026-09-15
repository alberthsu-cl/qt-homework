#include "pch.h"
#include "MainFrame.h"
#include "DemoController.h"
#include "DemoViewController.h"
#include "QtKitHost.h"

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

CMainFrame::CMainFrame()  = default;
CMainFrame::~CMainFrame() = default;

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

    // ---- build the layers, then start Qt ----------------------------------
    // Controller first: the view controller calls into it from the moment the
    // QML tree is up, which can be before Present() has returned.
    m_pController.reset(new CDemoController());
    m_pViewController.reset(new CDemoViewController(m_pController.get(), GetSafeHwnd()));

    if (!m_pViewController->Present())
    {
        m_wndStatus.SetWindowText(
            _T("  QtKit.dll failed to load - is this .exe in bin_x64\\PowerDirector\\ ?"));
    }

    return 0;
}

LRESULT CMainFrame::OnQmlClick(WPARAM wParam, LPARAM /*lParam*/)
{
    // Arrived via PostMessage from a Qt-thread callback. Safe to touch MFC now.
    //
    // By the time this runs the controller has already applied the rule and
    // republished to QML - the picture has moved. This handler only narrates.
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

        // MfcQmlDemo.exe "C:\path\to\picture.png" opens straight into an
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

    // Reading the numbers back out of the controller is the whole difference
    // this refactor makes: C++ can answer "what is the zoom?" without asking
    // QML, because C++ is the one that decided it.
    CString str;
    str.Format(_T("  QML \"%s\" -> C++ decided: zoom %.2fx, rot %d deg  (%d clicks)  <- native MFC strip"),
               pszWhat,
               m_pController->ZoomFactor(),
               m_pController->RotationDeg(),
               m_nClickCount);
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
    if (!m_pViewController)
        return;

    // Qt wants forward slashes and a file:/// URL.
    CString strUrl(strPath);
    strUrl.Replace(_T('\\'), _T('/'));
    strUrl = _T("file:///") + strUrl;

    CString strName(strPath);
    const int i = strName.ReverseFind(_T('\\'));
    if (i >= 0) strName = strName.Mid(i + 1);

    m_pViewController->PushImage(ToUtf8(strUrl), "C++ pushed:  " + ToUtf8(strName));

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

    // Geometry for the Qt child goes through QtKit so the scene reflows; the
    // view controller owns that hop.
    if (m_pViewController)
        m_pViewController->SetViewportSize(rc.Width(), rc.Height() - kStatusHeight);

    if (m_wndStatus.GetSafeHwnd())
    {
        m_wndStatus.SetWindowPos(nullptr, 0, rc.Height() - kStatusHeight,
                                 rc.Width(), kStatusHeight,
                                 SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void CMainFrame::OnDestroy()
{
    // Tear down in layer order: view controller detaches the Qt child and
    // unloads, then the Qt runloop stops, then the controller goes.
    if (m_pViewController)
    {
        m_pViewController->Dismiss();
        m_pViewController.reset();
    }
    QtKitHost::Inst().Stop();
    m_pController.reset();

    CFrameWnd::OnDestroy();
}
