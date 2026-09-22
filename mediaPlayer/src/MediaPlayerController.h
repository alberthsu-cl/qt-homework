#pragma once

#include <atomic>
#include <mutex>
#include <string>

class CMediaPlayerController
{
public:
    CMediaPlayerController();
    ~CMediaPlayerController();

    void SetImportedMedia(const std::string& mediaName);
    void TogglePlay();
    void Stop();
    void PublishState();

    bool HasMedia() const { return m_hasMedia.load(); }
    bool IsPlaying() const { return m_isPlaying.load(); }
    std::string MediaName() const;

private:
    std::atomic<bool> m_hasMedia{ false };
    std::atomic<bool> m_isPlaying{ false };
    mutable std::mutex m_mediaMutex;
    std::string m_mediaName;
};
