#pragma once
//
// CDemoViewController - a miniature of PDR's CQtEntryViewController
// (src/ui/common/Qt/QtViewController.h).
//
// Everything QML-shaped lives here and nowhere else: the .qml entry path, the
// adopted HWND, and the binding wiring. Compare PDR's CDeductCreditVC, whose
// entire body is a constructor naming the .qml plus an OnQmlDidLoad() that
// attaches two clicked actions - it holds no state and neither does this.
//
// The point of the layer is containment. Rename a QML id and exactly one C++
// file changes; CDemoController never learns that QML exists.
//
#include <windows.h>
#include <atomic>
#include <string>

class CDemoController;
struct IQmlContext;

class CDemoViewController
{
public:
    // hNotify receives WM_APP_QML_* messages; it is the MFC frame.
    CDemoViewController(CDemoController* pController, HWND hNotify);
    ~CDemoViewController();

    // Starts QtKit and loads the .qml. Returns false if QtKit.dll is missing.
    bool Present();

    // Detaches the Qt child and unloads. Safe to call twice.
    void Dismiss();

    bool IsReady()  const { return m_bQmlReady; }
    HWND QmlWnd()   const { return m_hQmlWnd; }

    // ---- callable from the MFC thread; each hops to the Qt thread itself ----
    void SetViewportSize(int cx, int cy);
    void PushImage(const std::string& strUrlUtf8, const std::string& strCaptionUtf8);
    void SynchronizeAppliedColor();
    void AddCurrentCustomColor();

private:
    // ---- these two run ON THE QT THREAD ------------------------------------
    void OnQtReady();
    void OnQmlDidLoad();

    void WireButtons(IQmlContext* pContext);

private:
    CDemoController* m_pController{ nullptr };   // not owned
    HWND             m_hNotify{ nullptr };       // the MFC frame, not owned

    // QML plumbing - the only state a view controller is allowed to keep.
    //
    // m_hQmlWnd and m_bQmlReady are atomic for the same reason CDemoController's
    // two values are: they are WRITTEN on the Qt thread (OnQmlDidLoad) and READ
    // on the MFC thread (OnSize -> SetViewportSize, the file dialog ->
    // PushImage), and Dismiss() writes them back from the MFC thread at
    // teardown. Neither is part of a larger invariant, so per-value atomicity
    // is enough - no lock needed.
    //
    // m_strQmlEntry needs no such treatment: it is written once on the Qt
    // thread before m_bQmlReady is ever set, and read only by Dismiss().
    std::atomic<HWND> m_hQmlWnd{ nullptr };
    std::string       m_strQmlEntry;
    std::atomic<bool> m_bQmlReady{ false };
};
