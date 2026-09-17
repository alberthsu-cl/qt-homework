#pragma once
#include <afxwin.h>
#include <memory>
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
enum QmlClick { kClickOpenPicker = 1, kClickAddCustom, kClickReset, kClickApply, kClickCancel };

class CDemoController;
class CDemoViewController;

// The MFC half of the demo, and nothing else: a menu, a status strip, a file
// dialog and a layout. It owns the controller, the controller owns the state,
// and the view controller owns everything QML-shaped.
//
// It used to own all three jobs at once - loading the .qml, adopting the HWND
// and wiring bindings all lived in this class, while the zoom and rotation it
// was reporting on actually lived in QML.
class CMainFrame : public CFrameWnd
{
public:
    CMainFrame();
    ~CMainFrame() override;   // out-of-line: the unique_ptr members are incomplete here

protected:
    afx_msg int  OnCreate(LPCREATESTRUCT lpcs);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDestroy();
    afx_msg void OnFileOpen();
    afx_msg void OnFileExit();
    afx_msg LRESULT OnQmlClick(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

private:
    void LayoutChildren();
    void PushImageToQml(const CString& strPath);

private:
    CStatic m_wndStatus;        // bottom strip, MFC-owned
    int     m_nClickCount{ 0 };

    std::unique_ptr<CDemoController>     m_pController;
    std::unique_ptr<CDemoViewController> m_pViewController;
};
