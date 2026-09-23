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
    WireSelectionProperty(context);
    WirePreviewGeometry(context);
    m_controller->PublishState();
    m_ready = true;
    ::PostMessage(m_notifyWindow, WM_APP_QML_CLICK, kClickReady, 0);
}

void CMediaPlayerViewController::WireSelectionProperty(IQmlContext* context)
{
    if (!context->isObjectBound(MediaPlayerName.property))
        return;

    context->property(MediaPlayerName.property).setPropertyChangedAction(
        "requestedMediaIndex", [this, context]() {
            if (!context->isObjectBound(MediaPlayerName.property))
                return;

            const int mediaIndex = context->property(MediaPlayerName.property)
                .propertyInt("requestedMediaIndex");
            if (mediaIndex < 0)
                return;

            m_requestedMediaIndex.store(mediaIndex);
            ::PostMessage(m_notifyWindow, WM_APP_QML_CLICK, kClickSelectMedia, 0);
        });
}

void CMediaPlayerViewController::WirePreviewGeometry(IQmlContext* context)
{
    if (!context->isObjectBound(MediaPlayerName.property))
        return;

    context->property(MediaPlayerName.property).setPropertyChangedAction(
        "previewRevision", [this, context]() {
            if (!context->isObjectBound(MediaPlayerName.property))
                return;

            auto& property = context->property(MediaPlayerName.property);
            const int x = property.propertyInt("previewX");
            const int y = property.propertyInt("previewY");
            const int width = property.propertyInt("previewWidth");
            const int height = property.propertyInt("previewHeight");
            QtKitHost::Log("[H2-03] QML preview rect: %d,%d %dx%d",
                x, y, width, height);

            ::PostMessage(m_notifyWindow, WM_APP_PREVIEW_SIZE,
                MAKELPARAM(x, y), MAKELPARAM(width, height));
        });
}

bool CMediaPlayerViewController::ApplyRequestedMediaSelection()
{
    const int mediaIndex = m_requestedMediaIndex.exchange(-1);
    if (mediaIndex < 0 || !m_controller->SelectMedia(static_cast<size_t>(mediaIndex)))
        return false;

    IQmlContext* context = QtKitHost::Inst().Context();
    if (context)
        context->runOnQtThread([this, context]() { UpdateSelectedMediaPreview(context); });
    return true;
}

void CMediaPlayerViewController::UpdateSelectedMediaPreview(IQmlContext* context)
{
    const SelectedMedia asset = m_controller->SelectedAsset();
    const SourceMediaInfo sourceInfo = m_controller->SelectedSourceInfo();
    if (context->isObjectBound(MediaPlayerName.previewImage))
    {
        const char* source = asset.kind == MediaKind::Image
            ? asset.sourceUrl.c_str() : "";
        context->image(MediaPlayerName.previewImage).source(source);
    }
    if (context->isObjectBound(MediaPlayerName.statusLabel))
    {
        std::string message = sourceInfo.statusText;
        if (sourceInfo.loaded && asset.kind == MediaKind::Image)
            message = "Image ready: " + asset.displayName + " - direct QML preview";
        else if (sourceInfo.loaded)
            message = "Ready: " + asset.displayName + " - MediaObj transport available";
        context->label(MediaPlayerName.statusLabel).text(message.c_str());
    }
}

void CMediaPlayerViewController::WireButtons(IQmlContext* context)
{
    struct ButtonAction
    {
        const char* name;
        QmlClick code;
    };

    const ButtonAction actions[] = {
        { MediaPlayerName.importButton, kClickImport },
        { MediaPlayerName.playButton, kClickPlay },
        { MediaPlayerName.stopButton, kClickStop },
    };

    for (const auto& item : actions)
    {
        if (!context->isObjectBound(item.name))
        {
            QtKitHost::Log("button not bound: %s", item.name);
            continue;
        }

        context->button(item.name).setClickedAction([this, item]() {
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
        if (context->isObjectBound(MediaPlayerName.property))
            context->property(MediaPlayerName.property)
                .property("previewLayoutReady", true);
    });
}

void CMediaPlayerViewController::PushMedia(
    const std::string& mediaPath,
    const std::string& mediaUrl,
    const std::string& mediaName)
{
    IQmlContext* context = QtKitHost::Inst().Context();
    if (!m_ready || !context)
        return;

    m_controller->SetImportedMedia(mediaPath, mediaUrl, mediaName);
    context->runOnQtThread([this, context]() { UpdateSelectedMediaPreview(context); });
}
