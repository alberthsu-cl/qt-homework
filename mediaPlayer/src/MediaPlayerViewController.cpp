#include "pch.h"
#include "MediaPlayerViewController.h"
#include "MainFrame.h"
#include "MediaPlayerController.h"
#include "MediaPlayerNames.h"
#include "QtKitHost.h"

CMediaPlayerViewController::CMediaPlayerViewController(
    CMediaPlayerController* controller,
    HWND notifyWindow)
    : m_controller(controller)
    , m_notifyWindow(notifyWindow)
{
}

CMediaPlayerViewController::~CMediaPlayerViewController()
{
    Dismiss();
}

bool CMediaPlayerViewController::Present()
{
    return QtKitHost::Inst().Start([this]() { OnQtReady(); });
}

void CMediaPlayerViewController::Dismiss()
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

void CMediaPlayerViewController::OnQtReady()
{
    if (!QtKitHost::Inst().Context())
        return;

    const std::string hostedQml = QtKitHost::Inst().ExeDir() +
        "MediaPlayerQml/MediaPlayerWindow.qml";
    if (::GetFileAttributesA(hostedQml.c_str()) != INVALID_FILE_ATTRIBUTES)
        m_qmlEntry = hostedQml;
    else
        m_qmlEntry = QtKitHost::Inst().ExeDir() + "../qml/MediaPlayerWindow.qml";
    QtKitHost::Inst().LoadQml(m_qmlEntry, [this]() { OnQmlDidLoad(); });
}

void CMediaPlayerViewController::OnQmlDidLoad()
{
    IQmlContext* context = QtKitHost::Inst().Context();
    if (!context || !context->isObjectBound(MediaPlayerName.window))
        return;

    HWND qmlWindow = nullptr;
    try
    {
        qmlWindow = static_cast<HWND>(context->window(MediaPlayerName.window).window());
    }
    catch (const QmlObjectNoBoundError& error)
    {
        QtKitHost::Log("window not bound: %s", error.what());
        return;
    }
    if (!qmlWindow)
        return;

    m_qmlWindow = qmlWindow;
    context->window(MediaPlayerName.window).setParent(m_notifyWindow);
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

void CMediaPlayerViewController::WireButtons(IQmlContext* context)
{
    using Action = void (CMediaPlayerController::*)();
    struct ButtonAction
    {
        const char* name;
        QmlClick code;
        Action action;
    };

    const ButtonAction actions[] = {
        { MediaPlayerName.importButton, kClickImport, nullptr },
        { MediaPlayerName.playButton, kClickPlay, &CMediaPlayerController::TogglePlay },
        { MediaPlayerName.stopButton, kClickStop, &CMediaPlayerController::Stop },
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

void CMediaPlayerViewController::SetViewportSize(int width, int height)
{
    IQmlContext* context = QtKitHost::Inst().Context();
    if (!m_ready || !context || !m_qmlWindow || !::IsWindow(m_qmlWindow))
        return;

    context->runOnQtThread([context, width, height]() {
        if (context->isObjectBound(MediaPlayerName.window))
            context->window(MediaPlayerName.window).setPosition(0, 0, width, height);
    });
}

void CMediaPlayerViewController::PushMedia(
    const std::string& mediaUrl,
    const std::string& mediaName)
{
    IQmlContext* context = QtKitHost::Inst().Context();
    if (!m_ready || !context)
        return;

    context->runOnQtThread([this, context, mediaUrl, mediaName]() {
        m_controller->SetImportedMedia(mediaName);
        if (context->isObjectBound(MediaPlayerName.previewImage))
            context->image(MediaPlayerName.previewImage).source(mediaUrl.c_str());
        if (context->isObjectBound(MediaPlayerName.statusLabel))
        {
            const std::string message = "Imported " + mediaName + " - add it to the timeline";
            context->label(MediaPlayerName.statusLabel).text(message.c_str());
        }
    });
}
