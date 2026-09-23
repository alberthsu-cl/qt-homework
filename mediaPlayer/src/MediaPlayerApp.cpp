#include "pch.h"
#include "MainFrame.h"
#include "PlaybackRuntimeProbe.h"

class CMediaPlayerApp : public CWinApp
{
public:
    BOOL InitInstance() override
    {
        ::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        CWinApp::InitInstance();

        if (__argc > 1 && __targv && __targv[1] &&
            _wcsicmp(__targv[1], L"--probe-playback-runtime") == 0)
        {
            wchar_t executablePath[MAX_PATH] = { 0 };
            ::GetModuleFileNameW(nullptr, executablePath, MAX_PATH);
            ::PathRemoveFileSpecW(executablePath);
            ::PathAddBackslashW(executablePath);
            const PlaybackRuntimeProbeResult probe = CPlaybackRuntimeProbe::Run(executablePath);
            ::ExitProcess(probe.IsReady() ? ERROR_SUCCESS : ERROR_OPEN_FAILED);
        }

        if (__argc > 2 && __targv && __targv[1] && __targv[2] &&
            (_wcsicmp(__targv[1], L"--probe-mediaobj") == 0 ||
             _wcsicmp(__targv[1], L"--probe-mediaobj-preview") == 0 ||
             _wcsicmp(__targv[1], L"--probe-mediaobj-transport") == 0))
        {
            wchar_t executablePath[MAX_PATH] = { 0 };
            ::GetModuleFileNameW(nullptr, executablePath, MAX_PATH);
            ::PathRemoveFileSpecW(executablePath);
            ::PathAddBackslashW(executablePath);
            const bool transportEnabled =
                _wcsicmp(__targv[1], L"--probe-mediaobj-transport") == 0;
            const bool previewEnabled = transportEnabled ||
                _wcsicmp(__targv[1], L"--probe-mediaobj-preview") == 0;
            const PlaybackRuntimeProbeResult probe = transportEnabled
                ? CPlaybackRuntimeProbe::RunTransportProbe(executablePath, __targv[2])
                : CPlaybackRuntimeProbe::RunSourceProbe(
                    executablePath, __targv[2], previewEnabled);
            const bool passed = probe.IsReady() && probe.sourceOpened &&
                (!transportEnabled || probe.transportPassed);
            ::ExitProcess(passed ? ERROR_SUCCESS : ERROR_OPEN_FAILED);
        }

        CMainFrame* frame = new CMainFrame;
        m_pMainWnd = frame;
        frame->Create(nullptr, _T("Media Player - Untitled Source"),
                      WS_OVERLAPPEDWINDOW, CRect(50, 50, 1490, 980));
        frame->ShowWindow(SW_SHOW);
        frame->UpdateWindow();
        return TRUE;
    }
};

CMediaPlayerApp theApp;
