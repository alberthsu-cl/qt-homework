#pragma once

#include <afxwin.h>
#include <memory>
#include <string>

#define IDM_FILE_IMPORT 1001
#define IDM_FILE_EXIT 1002
#define WM_APP_QML_CLICK (WM_APP + 1)
#define WM_APP_PREVIEW_SIZE (WM_APP + 2)

enum QmlClick
{
    kClickReady = 0,
    kClickImport,
    kClickPlay,
    kClickStop,
    kClickSelectMedia
};

class CMediaPlayerController;
class CMediaPlayerViewController;

class CMainFrame : public CFrameWnd
{
public:
    CMainFrame();
    ~CMainFrame() override;

protected:
    afx_msg int OnCreate(LPCREATESTRUCT createStruct);
    afx_msg void OnSize(UINT type, int width, int height);
    afx_msg void OnDestroy();
    afx_msg void OnFileImport();
    afx_msg void OnFileExit();
    afx_msg LRESULT OnQmlClick(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnPreviewSize(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

private:
    void LayoutChildren();
    void ImportMedia(const CString& path);

    CStatic m_status;
    HWND m_previewWindow{ nullptr };
    std::unique_ptr<CMediaPlayerController> m_controller;
    std::unique_ptr<CMediaPlayerViewController> m_viewController;
};
