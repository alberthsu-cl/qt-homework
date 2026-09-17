#pragma once

#include <atomic>
#include <string>
#include <windows.h>

class CDemoController;
struct IQmlContext;

// Owns the independent ColorPicker QML entry. Unlike CDemoViewController,
// this window remains top-level and is never reparented into the MFC frame.
class CColorPickerViewController
{
public:
    CColorPickerViewController(CDemoController* pController, HWND hNotify);
    ~CColorPickerViewController();

    void Present();
    void Dismiss();
    bool IsVisible() const { return m_bVisible; }

private:
    void OnQmlDidLoad();
    void WireProperty(IQmlContext* pContext);
    void WireButtons(IQmlContext* pContext);
    void Finish(bool bAccepted);
    void DismissOnQtThread();
    void PositionBesideHost(IQmlContext* pContext);

private:
    CDemoController* m_pController{ nullptr };
    HWND m_hNotify{ nullptr };
    std::string m_strQmlEntry;
    std::atomic<bool> m_bLoading{ false };
    std::atomic<bool> m_bVisible{ false };
};
