#pragma once

#include <cstdint>
#include <string>

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
    SourceMediaInfo CurrentInfo() const { return m_info; }

private:
    bool EnsureMediaObj();
    static std::wstring ToWide(const std::string& utf8Text);
    static std::string FormatHResult(long result);

    void* m_mediaObj{ nullptr };
    bool m_comInitialized{ false };
    SourceMediaInfo m_info;
};
