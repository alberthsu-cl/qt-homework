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
#include <string>
#include <vector>

class CDemoViewController;

class CDemoController
{
public:
    CDemoController();
    ~CDemoController();

    void BeginColorEdit();
    void SetPendingColorHex(const std::string& colorHex);
    void CommitAppliedColorHex(const std::string& colorHex);
    void ApplyColorEdit();
    void CancelColorEdit();
    void AddCustomColor();
    void ResetPendingColor();
    std::string AppliedColorHex() const;

    void PublishState();

private:
    static unsigned ParseColor(const std::string& colorHex, unsigned fallback);
    static std::string FormatColor(unsigned rgb);
    void LoadCustomColors();
    void SaveCustomColors() const;

    std::atomic<unsigned> m_appliedRgb{ 0x3B82F6 };
    std::atomic<unsigned> m_pendingRgb{ 0x3B82F6 };
    const unsigned m_initialRgb{ 0x3B82F6 };
    std::atomic<bool> m_isEditing{ false };
    std::vector<unsigned> m_customColors;
};
