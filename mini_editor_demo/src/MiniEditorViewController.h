#pragma once

#include <atomic>
#include <string>
#include <windows.h>

class CMiniEditorController;
struct IQmlContext;

class CMiniEditorViewController
{
public:
    CMiniEditorViewController(CMiniEditorController* controller, HWND notifyWindow);
    ~CMiniEditorViewController();

    bool Present();
    void Dismiss();
    void SetViewportSize(int width, int height);
    void PushMedia(const std::string& mediaUrl, const std::string& mediaName);

private:
    void OnQtReady();
    void OnQmlDidLoad();
    void WireButtons(IQmlContext* context);

    CMiniEditorController* m_controller{ nullptr };
    HWND m_notifyWindow{ nullptr };
    std::atomic<HWND> m_qmlWindow{ nullptr };
    std::atomic<bool> m_ready{ false };
    std::string m_qmlEntry;
};
