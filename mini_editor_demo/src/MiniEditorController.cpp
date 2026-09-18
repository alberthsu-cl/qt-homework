#include "pch.h"
#include "MiniEditorController.h"
#include "MiniEditorNames.h"
#include "QtKitHost.h"

CMiniEditorController::CMiniEditorController() = default;
CMiniEditorController::~CMiniEditorController() = default;

void CMiniEditorController::SetImportedMedia(const std::string& mediaName)
{
    {
        std::lock_guard<std::mutex> guard(m_mediaMutex);
        m_mediaName = mediaName;
    }
    m_hasMedia = true;
    m_hasTimelineClip = false;
    m_isPlaying = false;
    m_splitCount = 0;
    PublishState();
}

void CMiniEditorController::AddSelectedToTimeline()
{
    if (m_hasMedia)
        m_hasTimelineClip = true;
    PublishState();
}

void CMiniEditorController::TogglePlay()
{
    if (m_hasTimelineClip)
        m_isPlaying = !m_isPlaying.load();
    PublishState();
}

void CMiniEditorController::Stop()
{
    m_isPlaying = false;
    PublishState();
}

void CMiniEditorController::SplitAtPlayhead()
{
    if (m_hasTimelineClip)
        ++m_splitCount;
    PublishState();
}

void CMiniEditorController::ResetClipAdjustments()
{
    m_isPlaying = false;
    PublishState();
}

std::string CMiniEditorController::MediaName() const
{
    std::lock_guard<std::mutex> guard(m_mediaMutex);
    return m_mediaName;
}

void CMiniEditorController::PublishState()
{
    IQmlContext* context = QtKitHost::Inst().Context();
    if (!context || !context->isObjectBound(MiniEditorName.property))
        return;

    const std::string mediaName = MediaName();
    auto& property = context->property(MiniEditorName.property);
    property.property("mediaName", mediaName.c_str());
    property.property("hasMedia", m_hasMedia.load());
    property.property("hasTimelineClip", m_hasTimelineClip.load());
    property.property("playing", m_isPlaying.load());
    property.property("splitCount", m_splitCount.load());
}
