#pragma once

#include <string>

struct PlaybackRuntimeProbeResult
{
    bool runtimeFilesPresent{ false };
    bool comInitialized{ false };
    bool mediaObjActivated{ false };
    bool sourceOpened{ false };
    std::wstring statusText;

    bool IsReady() const
    {
        return runtimeFilesPresent && comInitialized && mediaObjActivated;
    }
};

class CPlaybackRuntimeProbe
{
public:
    // Runs on the MFC main thread. It verifies the MO-only package and proves
    // registration-free COM activation without retaining an engine interface.
    static PlaybackRuntimeProbeResult Run(const std::wstring& executableDirectory);

    // Opens and inspects one source without creating a preview window.
    static PlaybackRuntimeProbeResult RunSourceProbe(
        const std::wstring& executableDirectory,
        const std::wstring& sourcePath);
};
