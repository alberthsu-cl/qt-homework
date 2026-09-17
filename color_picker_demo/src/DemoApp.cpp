#include "pch.h"
#include "MainFrame.h"

// The whole MFC application object. Nothing here is QtKit-specific - the point
// is that a bog-standard CWinApp hosts Qt without knowing anything about it.
class CDemoApp : public CWinApp
{
public:
    virtual BOOL InitInstance() override
    {
        // Declare DPI awareness BEFORE any window exists.
        //
        // This matters more than it looks. Qt is per-monitor DPI aware and
        // always will be; if the MFC host is not, Windows virtualises the
        // host's coordinates (ask for 1000x680 on a 125% display, get 800x544)
        // while Qt keeps rendering the QML scene at the monitor's true scale.
        // The two then disagree about what a pixel is, and no amount of
        // SetWindowPos or IUIWindow::setPosition can reconcile them - the
        // scene renders larger than the window that contains it and anchored
        // content falls off the bottom edge.
        //
        // PDR is DPI-aware via its manifest and feeds QtKit the ratio it
        // computes in CDpiMgr. Here both sides are simply 1:1.
        ::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        CWinApp::InitInstance();

        CMainFrame* pFrame = new CMainFrame;
        m_pMainWnd = pFrame;

        pFrame->Create(nullptr, _T("QmlColorPiker - File > Open Preview Image..."),
                       WS_OVERLAPPEDWINDOW, CRect(80, 80, 1080, 760));
        pFrame->ShowWindow(SW_SHOW);
        pFrame->UpdateWindow();
        return TRUE;
    }
};

CDemoApp theApp;
