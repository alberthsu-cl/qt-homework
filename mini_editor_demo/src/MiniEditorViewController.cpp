#include "pch.h"
#include "MiniEditorViewController.h"
#include "MainFrame.h"
#include "MiniEditorController.h"
#include "MiniEditorNames.h"
#include "QtKitHost.h"

CMiniEditorViewController::CMiniEditorViewController(
    CMiniEditorController* controller,
    HWND notifyWindow)
    : m_controller(controller)
    , m_notifyWindow(notifyWindow)
{
}

CMiniEditorViewController::~CMiniEditorViewController()
{
    Dismiss();
}

bool CMiniEditorViewController::Present()
{
    return QtKitHost::Inst().Start([this]() { OnQtReady(); });
}

void CMiniEditorViewController::Dismiss()
{
    if (m_qmlWindow && ::IsWindow(m_qmlWindow))
    {
        ::SetParent(m_qmlWindow, nullptr);
        m_qmlWindow = nullptr;
    }
    if (!m_qmlEntry.empty())
    {
        QtKitHost::Inst().UnloadQml(m_qmlEntry);
        m_qmlEntry.clear();
    }
    m_ready = false;
}

void CMiniEditorViewController::OnQtReady()
{
    if (!QtKitHost::Inst().Context())
        return;

    m_qmlEntry = QtKitHost::Inst().ExeDir() + "../qml/MiniEditorWindow.qml";
    QtKitHost::Inst().LoadQml(m_qmlEntry, [this]() { OnQmlDidLoad(); });
}

void CMiniEditorViewController::OnQmlDidLoad()
{
    IQmlContext* context = QtKitHost::Inst().Context();
    if (!context || !context->isObjectBound(MiniEditorName.window))
        return;

    HWND qmlWindow = nullptr;
    try
    {
        qmlWindow = static_cast<HWND>(context->window(MiniEditorName.window).window());
    }
    catch (const QmlObjectNoBoundError& error)
    {
        QtKitHost::Log("window not bound: %s", error.what());
        return;
    }
    if (!qmlWindow)
        return;

    m_qmlWindow = qmlWindow;
    context->window(MiniEditorName.window).setParent(m_notifyWindow);
    ::SetWindowLongPtr(qmlWindow, GWL_STYLE,
        (::GetWindowLongPtr(qmlWindow, GWL_STYLE) | WS_CHILD)
        & ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX));
    ::SetWindowLongPtr(qmlWindow, GWL_EXSTYLE,
        ::GetWindowLongPtr(qmlWindow, GWL_EXSTYLE)
        & ~(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME));
    ::SetWindowPos(qmlWindow, nullptr, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    WireButtons(context);
    m_controller->PublishState();
    m_ready = true;
    ::PostMessage(m_notifyWindow, WM_APP_QML_CLICK, kClickReady, 0);
}

void CMiniEditorViewController::WireButtons(IQmlContext* context)
{
    using Action = void (CMiniEditorController::*)();
    struct ButtonAction
    {
        const char* name;
        QmlClick code;
        Action action;
    };

    const ButtonAction actions[] = {
        { MiniEditorName.importButton, kClickImport, nullptr },
        { MiniEditorName.addButton, kClickAdd, &CMiniEditorController::AddSelectedToTimeline },
        { MiniEditorName.playButton, kClickPlay, &CMiniEditorController::TogglePlay },
        { MiniEditorName.stopButton, kClickStop, &CMiniEditorController::Stop },
        { MiniEditorName.splitButton, kClickSplit, &CMiniEditorController::SplitAtPlayhead },
        { MiniEditorName.resetButton, kClickReset, &CMiniEditorController::ResetClipAdjustments },
    };

    for (const auto& item : actions)
    {
        if (!context->isObjectBound(item.name))
        {
            QtKitHost::Log("button not bound: %s", item.name);
            continue;
        }

        context->button(item.name).setClickedAction([this, item]() {
            if (item.action)
                (m_controller->*item.action)();
            ::PostMessage(m_notifyWindow, WM_APP_QML_CLICK,
                          static_cast<WPARAM>(item.code), 0);
        });
    }
}

void CMiniEditorViewController::SetViewportSize(int width, int height)
{
    IQmlContext* context = QtKitHost::Inst().Context();
    if (!m_ready || !context || !m_qmlWindow || !::IsWindow(m_qmlWindow))
        return;

    context->runOnQtThread([context, width, height]() {
        if (context->isObjectBound(MiniEditorName.window))
            context->window(MiniEditorName.window).setPosition(0, 0, width, height);
    });
}

void CMiniEditorViewController::PushMedia(
    const std::string& mediaUrl,
    const std::string& mediaName)
{
    IQmlContext* context = QtKitHost::Inst().Context();
    if (!m_ready || !context)
        return;

    context->runOnQtThread([this, context, mediaUrl, mediaName]() {
        m_controller->SetImportedMedia(mediaName);
        if (context->isObjectBound(MiniEditorName.previewImage))
            context->image(MiniEditorName.previewImage).source(mediaUrl.c_str());
        if (context->isObjectBound(MiniEditorName.statusLabel))
        {
            const std::string message = "Imported " + mediaName + " - add it to the timeline";
            context->label(MiniEditorName.statusLabel).text(message.c_str());
        }
    });
}
