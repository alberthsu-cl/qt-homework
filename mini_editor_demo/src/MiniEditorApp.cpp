#include "pch.h"
#include "MainFrame.h"

class CMiniEditorApp : public CWinApp
{
public:
    BOOL InitInstance() override
    {
        ::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        CWinApp::InitInstance();

        CMainFrame* frame = new CMainFrame;
        m_pMainWnd = frame;
        frame->Create(nullptr, _T("Mini Editor - Untitled Project"),
                      WS_OVERLAPPEDWINDOW, CRect(50, 50, 1490, 980));
        frame->ShowWindow(SW_SHOW);
        frame->UpdateWindow();
        return TRUE;
    }
};

CMiniEditorApp theApp;
