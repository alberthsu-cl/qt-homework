#include "pch.h"
#include "ColorPickerViewController.h"

#include "DemoController.h"
#include "DemoNames.h"
#include "MainFrame.h"
#include "QtKitHost.h"

#include <algorithm>

namespace {

const int kWindowGap = 16;

} // namespace

CColorPickerViewController::CColorPickerViewController(CDemoController* pController, HWND hNotify)
    : m_pController(pController)
    , m_hNotify(hNotify)
{
}

CColorPickerViewController::~CColorPickerViewController()
{
    Dismiss();
}

void CColorPickerViewController::Present()
{
    IQmlContext* pContext = QtKitHost::Inst().Context();
    if (!pContext || m_bVisible || m_bLoading.exchange(true))
        return;

    pContext->runOnQtThread([this]() {
        m_pController->BeginColorEdit();
        m_strQmlEntry = QtKitHost::Inst().ExeDir() + "../qml/ColorPickerDialog.qml";
        QtKitHost::Inst().LoadQml(m_strQmlEntry, [this]() { OnQmlDidLoad(); });
    });
}

void CColorPickerViewController::Dismiss()
{
    IQmlContext* pContext = QtKitHost::Inst().Context();
    if (!pContext || (!m_bVisible && !m_bLoading))
        return;

    // The owner may destroy this view controller immediately after Dismiss()
    // returns (for example while the MFC frame is closing). Wait until Qt has
    // unloaded the entry so no queued callback can retain a dangling `this`.
    pContext->runOnQtThreadSync([this]() { DismissOnQtThread(); });
}

void CColorPickerViewController::OnQmlDidLoad()
{
    IQmlContext* pContext = QtKitHost::Inst().Context();
    if (!pContext || !pContext->isObjectBound(DemoName.pickerWindow))
    {
        m_bLoading = false;
        return;
    }

    m_pController->PublishState();
    WireProperty(pContext);
    WireButtons(pContext);
    PositionBesideHost(pContext);
    m_bVisible = true;
    m_bLoading = false;
    QtKitHost::Log("independent ColorPicker window ready");
}

void CColorPickerViewController::WireProperty(IQmlContext* pContext)
{
    if (!pContext->isObjectBound(DemoName.pickerProperty))
        return;

    pContext->property(DemoName.pickerProperty).setPropertyChangedAction(
        "colorHex", [this, pContext]() {
            if (!pContext->isObjectBound(DemoName.pickerProperty))
                return;

            const std::string colorHex =
                pContext->property(DemoName.pickerProperty).propertyString("colorHex");
            m_pController->SetPendingColorHex(colorHex);
            QtKitHost::Log("live picker color -> %s", colorHex.c_str());

            // DemoWindow owns a different property binding. Bridge only the
            // live color so its preview can tint immediately while this
            // independent window is open.
            if (pContext->isObjectBound(DemoName.property))
                pContext->property(DemoName.property).property("colorHex", colorHex.c_str());
        });
}

void CColorPickerViewController::WireButtons(IQmlContext* pContext)
{
    if (pContext->isObjectBound(DemoName.addCustomButton))
    {
        pContext->button(DemoName.addCustomButton).setClickedAction([this, pContext]() {
            if (!pContext->isObjectBound(DemoName.pickerProperty))
                return;
            m_pController->SetPendingColorHex(
                pContext->property(DemoName.pickerProperty).propertyString("colorHex"));
            m_pController->AddCustomColor();
            ::PostMessage(m_hNotify, WM_APP_QML_CLICK, kClickAddCustom, 0);
        });
    }

    if (pContext->isObjectBound(DemoName.applyButton))
    {
        pContext->button(DemoName.applyButton).setClickedAction([this, pContext]() {
            if (!pContext->isObjectBound(DemoName.pickerProperty))
                return;
            m_pController->CommitAppliedColorHex(
                pContext->property(DemoName.pickerProperty).propertyString("colorHex"));
            m_pController->PublishState();
            ::PostMessage(m_hNotify, WM_APP_QML_CLICK, kClickApply, 0);
            pContext->postCall([this]() { Finish(true); });
        });
    }

    if (pContext->isObjectBound(DemoName.cancelButton))
    {
        pContext->button(DemoName.cancelButton).setClickedAction([this, pContext]() {
            m_pController->CancelColorEdit();
            ::PostMessage(m_hNotify, WM_APP_QML_CLICK, kClickCancel, 0);
            pContext->postCall([this]() { Finish(false); });
        });
    }
}

void CColorPickerViewController::Finish(bool /*bAccepted*/)
{
    DismissOnQtThread();
}

void CColorPickerViewController::DismissOnQtThread()
{
    if (!m_strQmlEntry.empty())
    {
        QtKitHost::Inst().UnloadQml(m_strQmlEntry);
        m_strQmlEntry.clear();
    }
    m_bVisible = false;
    m_bLoading = false;
}

void CColorPickerViewController::PositionBesideHost(IQmlContext* pContext)
{
    RECT hostRect = {};
    if (!::GetWindowRect(m_hNotify, &hostRect))
        return;

    HWND hPicker = nullptr;
    try
    {
        hPicker = static_cast<HWND>(pContext->window(DemoName.pickerWindow).window());
    }
    catch (const QmlObjectNoBoundError& e)
    {
        QtKitHost::Log("picker window not bound: %s", e.what());
        return;
    }

    RECT pickerRect = {};
    if (!hPicker || !::GetWindowRect(hPicker, &pickerRect))
        return;

    // The QML size is expressed in device-independent pixels. Qt has already
    // converted 560x420 to the correct native size for the current monitor
    // (700x525 at 125% DPI). Passing 560x420 to IUIWindow::setPosition here
    // resized the HWND in physical pixels and clipped the right and bottom
    // 20% of the scene. Move the native top-level window without resizing it.
    const int pickerWidth = pickerRect.right - pickerRect.left;
    const int pickerHeight = pickerRect.bottom - pickerRect.top;

    MONITORINFO monitorInfo = { sizeof(monitorInfo) };
    HMONITOR hMonitor = ::MonitorFromWindow(m_hNotify, MONITOR_DEFAULTTONEAREST);
    if (!::GetMonitorInfo(hMonitor, &monitorInfo))
        return;

    const RECT& work = monitorInfo.rcWork;
    int x = hostRect.right + kWindowGap;
    if (x + pickerWidth > work.right)
        x = hostRect.left - pickerWidth - kWindowGap;
    if (x < work.left)
        x = std::clamp(hostRect.left + 40, work.left, work.right - pickerWidth);

    int y = std::clamp(hostRect.top + 40, work.top, work.bottom - pickerHeight);
    ::SetWindowPos(hPicker, nullptr, x, y, 0, 0,
                   SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}
