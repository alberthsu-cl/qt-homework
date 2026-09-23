#include "pch.h"
#include "MainFrame.h"
#include "MediaPlayerController.h"
#include "MediaPlayerViewController.h"
#include "QtKitHost.h"

#include <afxdlgs.h>
#include <shlwapi.h>
#include <string>

namespace {

const int kStatusHeight = 28;

std::string ToUtf8(const CString& value)
{
    if (value.IsEmpty())
        return std::string();
    const int size = ::WideCharToMultiByte(CP_UTF8, 0, value, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 1)
        return std::string();
    std::string result(static_cast<size_t>(size), '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, value, -1, &result[0], size, nullptr, nullptr);
    result.pop_back();
    return result;
}

} // namespace

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_DESTROY()
    ON_COMMAND(IDM_FILE_IMPORT, &CMainFrame::OnFileImport)
    ON_COMMAND(IDM_FILE_EXIT, &CMainFrame::OnFileExit)
    ON_MESSAGE(WM_APP_QML_CLICK, &CMainFrame::OnQmlClick)
    ON_MESSAGE(WM_APP_PREVIEW_SIZE, &CMainFrame::OnPreviewSize)
END_MESSAGE_MAP()

CMainFrame::CMainFrame() = default;
CMainFrame::~CMainFrame() = default;

int CMainFrame::OnCreate(LPCREATESTRUCT createStruct)
{
    if (CFrameWnd::OnCreate(createStruct) == -1)
        return -1;

    CMenu menuBar;
    CMenu fileMenu;
    menuBar.CreateMenu();
    fileMenu.CreatePopupMenu();
    fileMenu.AppendMenu(MF_STRING, IDM_FILE_IMPORT, _T("&Import Media...\tCtrl+I"));
    fileMenu.AppendMenu(MF_SEPARATOR);
    fileMenu.AppendMenu(MF_STRING, IDM_FILE_EXIT, _T("E&xit"));
    menuBar.AppendMenu(MF_POPUP, reinterpret_cast<UINT_PTR>(fileMenu.Detach()), _T("&File"));
    SetMenu(&menuBar);
    menuBar.Detach();

    m_status.Create(_T("  Starting QtKit media player..."),
                    WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE,
                    CRect(0, 0, 0, 0), this, 0);
    m_status.SetFont(GetFont());

    m_previewWindow = ::CreateWindowExW(0, L"STATIC", nullptr,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SS_BLACKRECT,
        0, 0, 1, 1, GetSafeHwnd(), nullptr, AfxGetInstanceHandle(), nullptr);

    m_controller.reset(new CMediaPlayerController());
    m_controller->SetPreviewWindow(m_previewWindow);
    m_viewController.reset(new CMediaPlayerViewController(
        m_controller.get(), GetSafeHwnd(), m_previewWindow));
    if (!m_viewController->Present())
        m_status.SetWindowText(_T("  QtKit.dll failed to load from the executable folder."));

    return 0;
}

LRESULT CMainFrame::OnPreviewSize(WPARAM wParam, LPARAM lParam)
{
    if (m_controller)
        m_controller->ResizePreview(static_cast<int>(wParam), static_cast<int>(lParam));
    return 0;
}

LRESULT CMainFrame::OnQmlClick(WPARAM wParam, LPARAM)
{
    const QmlClick click = static_cast<QmlClick>(wParam);
    if (click == kClickReady)
    {
        LayoutChildren();
        m_status.SetWindowText(_T("  Media player ready - import media to begin."));
        if (__argc > 1 && __targv && __targv[1])
        {
            CString path(__targv[1]);
            path.Trim(_T('"'));
            if (::PathFileExists(path))
                ImportMedia(path);
        }
        return 0;
    }

    if (click == kClickImport)
    {
        OnFileImport();
        return 0;
    }

    if (click == kClickSelectMedia)
    {
        m_viewController->ApplyRequestedMediaSelection();
        return 0;
    }

    CString text;
    switch (click)
    {
    case kClickPlay:
        text.Format(_T("  Preview: %s"), m_controller->IsPlaying() ? _T("playing") : _T("paused"));
        break;
    case kClickStop:
        text = _T("  Preview stopped.");
        break;
    default:
        break;
    }
    m_status.SetWindowText(text);
    return 0;
}

void CMainFrame::OnFileImport()
{
    CFileDialog dialog(TRUE, nullptr, nullptr,
        OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
        _T("Media|*.mp4;*.mov;*.mkv;*.avi;*.mp3;*.wav;*.png;*.jpg;*.jpeg;*.bmp|All files|*.*||"),
        this);
    if (dialog.DoModal() == IDOK)
        ImportMedia(dialog.GetPathName());
}

void CMainFrame::ImportMedia(const CString& path)
{
    CString url(path);
    url.Replace(_T('\\'), _T('/'));
    url = _T("file:///") + url;

    CString name(path);
    const int slash = name.ReverseFind(_T('\\'));
    if (slash >= 0)
        name = name.Mid(slash + 1);

    m_viewController->PushMedia(ToUtf8(path), ToUtf8(url), ToUtf8(name));
    SetWindowText(_T("Media Player - ") + name);
    m_status.SetWindowText(_T("  Imported ") + name);
}

void CMainFrame::OnFileExit()
{
    PostMessage(WM_CLOSE);
}

void CMainFrame::OnSize(UINT type, int width, int height)
{
    CFrameWnd::OnSize(type, width, height);
    LayoutChildren();
}

void CMainFrame::LayoutChildren()
{
    CRect client;
    GetClientRect(&client);
    if (client.IsRectEmpty())
        return;

    if (m_viewController)
        m_viewController->SetViewportSize(client.Width(), client.Height() - kStatusHeight);
    if (m_status.GetSafeHwnd())
    {
        m_status.SetWindowPos(nullptr, 0, client.Height() - kStatusHeight,
                              client.Width(), kStatusHeight,
                              SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void CMainFrame::OnDestroy()
{
    if (m_controller)
        m_controller->SetPreviewWindow(nullptr);
    if (m_viewController)
    {
        m_viewController->Dismiss();
        m_viewController.reset();
    }
    QtKitHost::Inst().Stop();
    m_controller.reset();
    if (m_previewWindow && ::IsWindow(m_previewWindow))
        ::DestroyWindow(m_previewWindow);
    m_previewWindow = nullptr;
    CFrameWnd::OnDestroy();
}
