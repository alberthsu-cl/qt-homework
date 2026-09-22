#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>

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
    void TogglePlay();
    void Stop();
    void PublishState();

    bool HasMedia() const { return m_hasMedia.load(); }
    bool IsPlaying() const { return m_isPlaying.load(); }
    SelectedMedia SelectedAsset() const;

private:
    static MediaKind ClassifyMedia(const std::string& filePath);
    static std::string CreateStableMediaId(const std::string& filePath);
    static const char* MediaKindText(MediaKind kind);

    std::atomic<bool> m_hasMedia{ false };
    std::atomic<bool> m_isPlaying{ false };
    mutable std::mutex m_mediaMutex;
    SelectedMedia m_selectedMedia;
};
