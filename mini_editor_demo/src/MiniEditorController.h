#pragma once

#include <atomic>
#include <mutex>
#include <string>

class CMiniEditorController
{
public:
    CMiniEditorController();
    ~CMiniEditorController();

    void SetImportedMedia(const std::string& mediaName);
    void AddSelectedToTimeline();
    void TogglePlay();
    void Stop();
    void SplitAtPlayhead();
    void ResetClipAdjustments();
    void PublishState();

    bool HasMedia() const { return m_hasMedia.load(); }
    bool HasTimelineClip() const { return m_hasTimelineClip.load(); }
    bool IsPlaying() const { return m_isPlaying.load(); }
    int SplitCount() const { return m_splitCount.load(); }
    std::string MediaName() const;

private:
    std::atomic<bool> m_hasMedia{ false };
    std::atomic<bool> m_hasTimelineClip{ false };
    std::atomic<bool> m_isPlaying{ false };
    std::atomic<int> m_splitCount{ 0 };
    mutable std::mutex m_mediaMutex;
    std::string m_mediaName;
};
