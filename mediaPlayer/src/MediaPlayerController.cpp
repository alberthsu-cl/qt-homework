#include "pch.h"
#include "MediaPlayerController.h"
#include "MediaPlayerNames.h"
#include "QtKitHost.h"

#include <algorithm>
#include <cctype>

CMediaPlayerController::CMediaPlayerController() = default;
CMediaPlayerController::~CMediaPlayerController() = default;

void CMediaPlayerController::SetImportedMedia(const std::string& filePath,
                                              const std::string& sourceUrl,
                                              const std::string& displayName)
{
    {
        std::lock_guard<std::mutex> guard(m_mediaMutex);
        m_selectedMedia.id = CreateStableMediaId(filePath);
        m_selectedMedia.filePath = filePath;
        m_selectedMedia.sourceUrl = sourceUrl;
        m_selectedMedia.displayName = displayName;
        m_selectedMedia.kind = ClassifyMedia(filePath);
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

SelectedMedia CMediaPlayerController::SelectedAsset() const
{
    std::lock_guard<std::mutex> guard(m_mediaMutex);
    return m_selectedMedia;
}

MediaKind CMediaPlayerController::ClassifyMedia(const std::string& filePath)
{
    const size_t dot = filePath.find_last_of('.');
    if (dot == std::string::npos)
        return MediaKind::Unknown;

    std::string extension = filePath.substr(dot + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char value) { return static_cast<char>(std::tolower(value)); });

    if (extension == "mp4" || extension == "mov" || extension == "mkv" ||
        extension == "avi" || extension == "wmv" || extension == "mpeg" ||
        extension == "mpg")
        return MediaKind::Video;
    if (extension == "mp3" || extension == "wav" || extension == "m4a" ||
        extension == "aac" || extension == "flac" || extension == "wma")
        return MediaKind::Audio;
    if (extension == "png" || extension == "jpg" || extension == "jpeg" ||
        extension == "bmp" || extension == "gif" || extension == "tif" ||
        extension == "tiff" || extension == "webp")
        return MediaKind::Image;
    return MediaKind::Unknown;
}

std::string CMediaPlayerController::CreateStableMediaId(const std::string& filePath)
{
    constexpr uint64_t kFnvOffsetBasis = 14695981039346656037ull;
    constexpr uint64_t kFnvPrime = 1099511628211ull;
    uint64_t hash = kFnvOffsetBasis;
    for (unsigned char value : filePath)
    {
        const unsigned char normalized = value == '\\'
            ? '/' : static_cast<unsigned char>(std::tolower(value));
        hash ^= normalized;
        hash *= kFnvPrime;
    }
    return "asset-" + std::to_string(hash);
}

const char* CMediaPlayerController::MediaKindText(MediaKind kind)
{
    switch (kind)
    {
    case MediaKind::Video: return "Video";
    case MediaKind::Audio: return "Audio";
    case MediaKind::Image: return "Image";
    default: return "Unknown";
    }
}

void CMediaPlayerController::PublishState()
{
    IQmlContext* context = QtKitHost::Inst().Context();
    if (!context || !context->isObjectBound(MediaPlayerName.property))
        return;

    const SelectedMedia asset = SelectedAsset();
    const std::string mediaKind = MediaKindText(asset.kind);
    const bool hasMedia = m_hasMedia.load();
    const bool playing = m_isPlaying.load();
    context->runOnQtThread([context, asset, mediaKind, hasMedia, playing]() {
        if (!context->isObjectBound(MediaPlayerName.property))
            return;

        auto& property = context->property(MediaPlayerName.property);
        property.property("mediaId", asset.id.c_str());
        property.property("mediaPath", asset.filePath.c_str());
        property.property("mediaName", asset.displayName.c_str());
        property.property("mediaKind", mediaKind.c_str());
        property.property("hasMedia", hasMedia);
        property.property("playing", playing);
    });
}
