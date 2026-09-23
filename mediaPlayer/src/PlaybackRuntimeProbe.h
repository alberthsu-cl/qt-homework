#pragma once

#include <string>

struct PlaybackRuntimeProbeResult
{
    bool runtimeFilesPresent{ false };
    bool comInitialized{ false };
    bool mediaObjActivated{ false };
    bool sourceOpened{ false };
    bool transportPassed{ false };
    std::wstring statusText;

    bool IsReady() const
    {
        return runtimeFilesPresent && comInitialized && mediaObjActivated;
    }
};

class CPlaybackRuntimeProbe
{
public:
    // Runs on the MFC main thread. It verifies the MediaObj source package and proves
    // registration-free COM activation without retaining an engine interface.
    static PlaybackRuntimeProbeResult Run(const std::wstring& executableDirectory);

    // Opens and inspects one source. Preview mode verifies the EVR graph
    // against a private native window without starting the application UI.
    static PlaybackRuntimeProbeResult RunSourceProbe(
        const std::wstring& executableDirectory,
        const std::wstring& sourcePath,
        bool previewEnabled = false);

    // Loads a preview graph and exercises play, position, pause, and stop on
    // the same adapter used by the application controller.
    static PlaybackRuntimeProbeResult RunTransportProbe(
        const std::wstring& executableDirectory,
        const std::wstring& sourcePath);
};
