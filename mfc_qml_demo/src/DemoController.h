#pragma once
//
// CDemoController - a miniature of PDR's CQtController / CQtModalController
// (src/ui/common/Qt/QtController.h).
//
// This is the layer the demo used to be missing. It owns the feature's STATE
// and the RULES over that state; it knows nothing about QML object ids, .qml
// paths or HWNDs - that is the view controller's job next door.
//
// The split matters because PDR's real controllers are 380 files against 96
// view controllers: most of them never front QML at all. Keeping the rules
// here is what lets the same controller be driven by a QML button today and a
// menu item, a hotkey or a unit test tomorrow.
//
// Lives for the whole session, owned by CMainFrame.
//
#include <atomic>

class CDemoViewController;

class CDemoController
{
public:
    CDemoController();
    ~CDemoController();

    // The rules. Each one clamps, stores, and republishes to QML.
    //
    // These run ON THE QT THREAD - they are reached from a button's clicked
    // action. That is convenient rather than accidental: publishing touches
    // IUIProperty, which must happen on the Qt thread anyway.
    void ZoomIn();
    void ZoomOut();
    void Rotate90();
    void ResetView();

    float ZoomFactor()  const { return m_fZoom; }
    int   RotationDeg() const { return m_nRotation; }

    // Pushes current state into qml/DemoProperty.qml. Call once after the QML
    // tree is up, then after every mutation.
    void PublishState();

private:
    // atomic because the MFC thread may read these for the status strip while
    // the Qt thread is writing them. Neither value is part of a larger
    // invariant, so per-value atomicity is enough - no lock needed.
    std::atomic<float> m_fZoom{ 1.0f };
    std::atomic<int>   m_nRotation{ 0 };
};
