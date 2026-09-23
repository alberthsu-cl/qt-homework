#pragma once

#include <cstdint>
#include <string>
#include <windows.h>

struct SourceMediaInfo
{
    bool loaded{ false };
    bool hasDuration{ false };
    bool hasDimensions{ false };
    uint32_t mediaType{ 0 };
    uint32_t width{ 0 };
    uint32_t height{ 0 };
    int64_t duration100ns{ 0 };
    long errorCode{ 0 };
    std::string statusText;
};

// Owns the registration-free MediaObj COM object on the MFC UI thread. This
// is the only Homework-2 class that knows the proprietary interface ABI.
class CMediaObjSourceAdapter
{
public:
    CMediaObjSourceAdapter();
    ~CMediaObjSourceAdapter();

    SourceMediaInfo Load(const std::string& utf8Path);
    void Unload();
    bool Play();
    bool Pause();
    bool Stop();
    bool GetCurrentPosition(int64_t& position100ns);
    void SetPreviewWindow(HWND window);
    void ResizePreview(int x, int y, int width, int height);
    SourceMediaInfo CurrentInfo() const { return m_info; }

private:
    bool EnsureMediaObj();
    bool ApplyTransportResult(long result, const char* successText,
                              const char* failureText);
    static std::wstring ToWide(const std::string& utf8Text);
    static std::string FormatHResult(long result);

    void* m_mediaObj{ nullptr };
    HWND m_previewWindow{ nullptr };
    RECT m_previewBounds{ 0, 0, 1, 1 };
    bool m_comInitialized{ false };
    SourceMediaInfo m_info;
};
