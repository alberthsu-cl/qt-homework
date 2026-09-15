#pragma once
#include <afxwin.h>
#include <string>

// Menu command ids (the menu is built in code - no .rc file in this demo).
#define IDM_FILE_OPEN   1001
#define IDM_FILE_EXIT   1002

// Qt thread -> MFC main thread marshalling.
//
// A QtKit callback arrives on the QT THREAD. Touching a CWnd from there is
// undefined behaviour. PDR's answer is CQtController::RunOnMainThreadAsync(),
// which posts through CMsgProxy so the call rides the MFC message pump.
// This demo does the same thing with one PostMessage.
#define WM_APP_QML_CLICK  (WM_APP + 1)

// wParam values for WM_APP_QML_CLICK - which QML button was pressed.
enum QmlClick { kClickZoomIn = 1, kClickZoomOut, kClickRotate, kClickReset };

class CMainFrame : public CFrameWnd
{
public:
    CMainFrame();

protected:
    afx_msg int  OnCreate(LPCREATESTRUCT lpcs);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDestroy();
    afx_msg void OnFileOpen();
    afx_msg void OnFileExit();
    afx_msg LRESULT OnQmlClick(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

private:
    // Runs on the Qt thread once IQmlContext exists: kicks off the .qml load.
    void OnQtReady();

    // Runs on the Qt thread after the .qml tree is fully built: pulls the QML
    // window's HWND out, reparents it, and wires every binding.
    void AdoptQmlWindow();

    void LayoutChildren();
    void PushImageToQml(const CString& strPath);

private:
    CStatic     m_wndStatus;            // bottom strip, MFC-owned
    HWND        m_hQmlWnd{ nullptr };   // the QQuickWindow, now our child
    std::string m_strQmlEntry;          // absolute path of the loaded .qml
    bool        m_bQmlReady{ false };
    int         m_nClickCount{ 0 };
};
