#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "MediaObjSourceAdapter.h"

enum class MediaKind
{
    Unknown,
    Video,
    Audio,
    Image
};

struct SelectedMedia
{
    std::string id;
    std::string filePath;
    std::string sourceUrl;
    std::string displayName;
    MediaKind kind{ MediaKind::Unknown };
};

class CMediaPlayerController
{
public:
    CMediaPlayerController();
    ~CMediaPlayerController();

    void SetImportedMedia(const std::string& filePath,
                          const std::string& sourceUrl,
                          const std::string& displayName);
    bool SelectMedia(size_t catalogIndex);
    void TogglePlay();
    void Stop();
    void RefreshPlaybackPosition();
    void SetPreviewWindow(HWND window);
    void ResizePreview(int x, int y, int width, int height);
    void PublishState();

    bool HasMedia() const { return m_hasMedia.load(); }
    bool IsPlaying() const { return m_isPlaying.load(); }
    SelectedMedia SelectedAsset() const;
    std::vector<SelectedMedia> Catalog() const;
    SourceMediaInfo SelectedSourceInfo() const;

private:
    static MediaKind ClassifyMedia(const std::string& filePath);
    static std::string CreateStableMediaId(const std::string& filePath);
    static const char* MediaKindText(MediaKind kind);
    static std::string CreateCatalogJson(const std::vector<SelectedMedia>& catalog);
    void LoadSelectedSource(const SelectedMedia& asset);

    std::atomic<bool> m_hasMedia{ false };
    std::atomic<bool> m_isPlaying{ false };
    std::atomic<int64_t> m_position100ns{ 0 };
    mutable std::mutex m_mediaMutex;
    std::vector<SelectedMedia> m_catalog;
    size_t m_selectedIndex{ 0 };
    SelectedMedia m_selectedMedia;
    SourceMediaInfo m_selectedSourceInfo;
    CMediaObjSourceAdapter m_sourceAdapter;
};
