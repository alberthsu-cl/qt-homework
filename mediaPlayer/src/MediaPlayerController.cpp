#include "pch.h"
#include "MediaPlayerController.h"
#include "MediaPlayerNames.h"
#include "QtKitHost.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <sstream>

CMediaPlayerController::CMediaPlayerController() = default;
CMediaPlayerController::~CMediaPlayerController() = default;

void CMediaPlayerController::SetImportedMedia(const std::string& filePath,
                                              const std::string& sourceUrl,
                                              const std::string& displayName)
{
    SelectedMedia selectedAsset;
    {
        std::lock_guard<std::mutex> guard(m_mediaMutex);
        SelectedMedia asset;
        asset.id = CreateStableMediaId(filePath);
        asset.filePath = filePath;
        asset.sourceUrl = sourceUrl;
        asset.displayName = displayName;
        asset.kind = ClassifyMedia(filePath);

        const auto existing = std::find_if(m_catalog.begin(), m_catalog.end(),
            [&asset](const SelectedMedia& item) { return item.id == asset.id; });
        if (existing == m_catalog.end())
        {
            m_catalog.push_back(asset);
            m_selectedIndex = m_catalog.size() - 1;
        }
        else
        {
            *existing = asset;
            m_selectedIndex = static_cast<size_t>(std::distance(m_catalog.begin(), existing));
        }
        m_selectedMedia = m_catalog[m_selectedIndex];
        selectedAsset = m_selectedMedia;
    }
    m_hasMedia = true;
    m_isPlaying = false;
    LoadSelectedSource(selectedAsset);
    PublishState();
}

bool CMediaPlayerController::SelectMedia(size_t catalogIndex)
{
    bool selected = false;
    SelectedMedia selectedAsset;
    {
        std::lock_guard<std::mutex> guard(m_mediaMutex);
        if (catalogIndex < m_catalog.size())
        {
            m_selectedIndex = catalogIndex;
            m_selectedMedia = m_catalog[m_selectedIndex];
            selectedAsset = m_selectedMedia;
            selected = true;
        }
    }

    if (selected)
    {
        m_isPlaying = false;
        LoadSelectedSource(selectedAsset);
        PublishState();
    }
    return selected;
}

void CMediaPlayerController::TogglePlay()
{
    const SelectedMedia asset = SelectedAsset();
    const SourceMediaInfo sourceInfo = SelectedSourceInfo();
    if (!m_hasMedia || asset.kind == MediaKind::Image || !sourceInfo.loaded)
        return;

    const bool wasPlaying = m_isPlaying.load();
    const bool succeeded = wasPlaying
        ? m_sourceAdapter.Pause() : m_sourceAdapter.Play();
    if (succeeded)
        m_isPlaying = !wasPlaying;

    {
        std::lock_guard<std::mutex> guard(m_mediaMutex);
        m_selectedSourceInfo = m_sourceAdapter.CurrentInfo();
    }
    RefreshPlaybackPosition();
}

void CMediaPlayerController::Stop()
{
    const SelectedMedia asset = SelectedAsset();
    const SourceMediaInfo sourceInfo = SelectedSourceInfo();
    if (!m_hasMedia || asset.kind == MediaKind::Image || !sourceInfo.loaded)
        return;

    if (m_sourceAdapter.Stop())
    {
        m_isPlaying = false;
        m_position100ns = 0;
    }
    {
        std::lock_guard<std::mutex> guard(m_mediaMutex);
        m_selectedSourceInfo = m_sourceAdapter.CurrentInfo();
    }
    PublishState();
}

void CMediaPlayerController::RefreshPlaybackPosition()
{
    int64_t position100ns = 0;
    if (m_sourceAdapter.GetCurrentPosition(position100ns))
        m_position100ns = position100ns;
    PublishState();
}

void CMediaPlayerController::SetPreviewWindow(HWND window)
{
    m_sourceAdapter.SetPreviewWindow(window);
}

void CMediaPlayerController::ResizePreview(int x, int y, int width, int height)
{
    m_sourceAdapter.ResizePreview(x, y, width, height);
}

SelectedMedia CMediaPlayerController::SelectedAsset() const
{
    std::lock_guard<std::mutex> guard(m_mediaMutex);
    return m_selectedMedia;
}

std::vector<SelectedMedia> CMediaPlayerController::Catalog() const
{
    std::lock_guard<std::mutex> guard(m_mediaMutex);
    return m_catalog;
}

SourceMediaInfo CMediaPlayerController::SelectedSourceInfo() const
{
    std::lock_guard<std::mutex> guard(m_mediaMutex);
    return m_selectedSourceInfo;
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

std::string CMediaPlayerController::CreateCatalogJson(const std::vector<SelectedMedia>& catalog)
{
    const auto escapeJson = [](const std::string& value) {
        std::ostringstream stream;
        for (const unsigned char character : value)
        {
            switch (character)
            {
            case '\\': stream << "\\\\"; break;
            case '\"': stream << "\\\""; break;
            case '\n': stream << "\\n"; break;
            case '\r': stream << "\\r"; break;
            case '\t': stream << "\\t"; break;
            default:
                if (character < 0x20)
                    stream << " ";
                else
                    stream << static_cast<char>(character);
                break;
            }
        }
        return stream.str();
    };

    std::ostringstream stream;
    stream << "[";
    for (size_t index = 0; index < catalog.size(); ++index)
    {
        const SelectedMedia& asset = catalog[index];
        if (index != 0)
            stream << ",";
        stream << "{\"id\":\"" << escapeJson(asset.id)
               << "\",\"name\":\"" << escapeJson(asset.displayName)
               << "\",\"kind\":\"" << MediaKindText(asset.kind)
               << "\",\"source\":\"" << escapeJson(asset.sourceUrl)
               << "\"}";
    }
    stream << "]";
    return stream.str();
}

void CMediaPlayerController::LoadSelectedSource(const SelectedMedia& asset)
{
    m_position100ns = 0;
    SourceMediaInfo sourceInfo;
    if (asset.kind == MediaKind::Image)
    {
        m_sourceAdapter.Unload();
        sourceInfo.loaded = true;
        sourceInfo.statusText = "Image source ready for QML preview.";
    }
    else
    {
        sourceInfo = m_sourceAdapter.Load(asset.filePath);
    }

    QtKitHost::Log("[H2-03] source route: kind=%s, loaded=%s, error=0x%08lX, status=%s",
        MediaKindText(asset.kind), sourceInfo.loaded ? "yes" : "no",
        static_cast<unsigned long>(sourceInfo.errorCode), sourceInfo.statusText.c_str());

    std::lock_guard<std::mutex> guard(m_mediaMutex);
    m_selectedSourceInfo = sourceInfo;
}

void CMediaPlayerController::PublishState()
{
    IQmlContext* context = QtKitHost::Inst().Context();
    if (!context || !context->isObjectBound(MediaPlayerName.property))
        return;

    const SelectedMedia asset = SelectedAsset();
    const std::string catalogJson = CreateCatalogJson(Catalog());
    const SourceMediaInfo sourceInfo = SelectedSourceInfo();
    const std::string mediaKind = MediaKindText(asset.kind);
    const bool hasMedia = m_hasMedia.load();
    const bool playing = m_isPlaying.load();
    const int positionMs = static_cast<int>(std::min<int64_t>(
        m_position100ns.load() / 10000, INT_MAX));
    const int durationMs = static_cast<int>(std::min<int64_t>(
        sourceInfo.duration100ns / 10000, INT_MAX));
    context->runOnQtThread([context, asset, catalogJson, mediaKind, sourceInfo,
                            hasMedia, playing, positionMs, durationMs]() {
        if (!context->isObjectBound(MediaPlayerName.property))
            return;

        auto& property = context->property(MediaPlayerName.property);
        property.property("mediaId", asset.id.c_str());
        property.property("mediaPath", asset.filePath.c_str());
        property.property("mediaName", asset.displayName.c_str());
        property.property("mediaKind", mediaKind.c_str());
        property.property("mediaCatalogJson", catalogJson.c_str());
        property.property("mediaObjLoaded", sourceInfo.loaded);
        property.property("mediaLoadStatus", sourceInfo.statusText.c_str());
        property.property("mediaLoadError", static_cast<int>(sourceInfo.errorCode));
        property.property("hasMedia", hasMedia);
        property.property("playing", playing);
        property.property("positionMs", positionMs);
        property.property("durationMs", durationMs);
        if (context->isObjectBound(MediaPlayerName.statusLabel))
            context->label(MediaPlayerName.statusLabel).text(sourceInfo.statusText.c_str());
    });
}
