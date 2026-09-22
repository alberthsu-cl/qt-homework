#include "pch.h"
#include "MediaPlayerController.h"
#include "MediaPlayerNames.h"
#include "QtKitHost.h"

CMediaPlayerController::CMediaPlayerController() = default;
CMediaPlayerController::~CMediaPlayerController() = default;

void CMediaPlayerController::SetImportedMedia(const std::string& mediaName)
{
    {
        std::lock_guard<std::mutex> guard(m_mediaMutex);
        m_mediaName = mediaName;
    }
    m_hasMedia = true;
    m_isPlaying = false;
    PublishState();
}

void CMediaPlayerController::TogglePlay()
{
    if (m_hasMedia)
        m_isPlaying = !m_isPlaying.load();
    PublishState();
}

void CMediaPlayerController::Stop()
{
    m_isPlaying = false;
    PublishState();
}

std::string CMediaPlayerController::MediaName() const
{
    std::lock_guard<std::mutex> guard(m_mediaMutex);
    return m_mediaName;
}

void CMediaPlayerController::PublishState()
{
    IQmlContext* context = QtKitHost::Inst().Context();
    if (!context || !context->isObjectBound(MediaPlayerName.property))
        return;

    const std::string mediaName = MediaName();
    auto& property = context->property(MediaPlayerName.property);
    property.property("mediaName", mediaName.c_str());
    property.property("hasMedia", m_hasMedia.load());
    property.property("playing", m_isPlaying.load());
}
