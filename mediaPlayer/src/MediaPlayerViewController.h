#pragma once

#include <atomic>
#include <string>
#include <windows.h>

class CMediaPlayerController;
struct IQmlContext;

class CMediaPlayerViewController
{
public:
    CMediaPlayerViewController(CMediaPlayerController* controller, HWND notifyWindow);
    ~CMediaPlayerViewController();

    bool Present();
    void Dismiss();
    void SetViewportSize(int width, int height);
    void PushMedia(const std::string& mediaPath,
                   const std::string& mediaUrl,
                   const std::string& mediaName);
    bool ApplyRequestedMediaSelection();

private:
    void OnQtReady();
    void OnQmlDidLoad();
    void WireButtons(IQmlContext* context);
    void WireSelectionProperty(IQmlContext* context);
    void UpdateSelectedMediaPreview(IQmlContext* context);

    CMediaPlayerController* m_controller{ nullptr };
    HWND m_notifyWindow{ nullptr };
    std::atomic<HWND> m_qmlWindow{ nullptr };
    std::atomic<bool> m_ready{ false };
    std::atomic<int> m_requestedMediaIndex{ -1 };
    std::string m_qmlEntry;
};
