#pragma once

#include <string>

struct PlaybackRuntimeProbeResult
{
    bool runtimeFilesPresent{ false };
    bool comInitialized{ false };
    bool mediaObjActivated{ false };
    bool simbaActivated{ false };
    std::wstring statusText;

    bool IsReady() const
    {
        return runtimeFilesPresent && comInitialized && mediaObjActivated && simbaActivated;
    }
};

class CPlaybackRuntimeProbe
{
public:
    // Runs on the MFC main thread. It verifies the staged files and proves
    // COM activation only; it never retains an engine interface.
    static PlaybackRuntimeProbeResult Run(const std::wstring& executableDirectory);

    // Internal child-process entry point. SIMBA can fault during construction
    // when its broader PDR runtime context is incomplete, so the main UI never
    // executes this probe in-process.
    static int RunSimbaChild();
};
