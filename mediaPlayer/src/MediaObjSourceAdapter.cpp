#include "pch.h"
#include "MediaObjSourceAdapter.h"

#include <objbase.h>
#include <sstream>
#include <vector>

#pragma comment(lib, "Ole32.lib")

namespace {

const GUID kClsidMediaObj =
{ 0x302147f3, 0x0a17, 0x4fff, { 0x89, 0x78, 0xf8, 0xb7, 0xa6, 0x4f, 0xcd, 0xa7 } };
const GUID kIidMediaObj13 =
{ 0x35eb2c0c, 0x373f, 0x4abb, { 0xaa, 0xe1, 0x62, 0x25, 0xcb, 0x35, 0x8e, 0xcc } };
const GUID kIidClRegPath =
{ 0xebb44941, 0xac60, 0x49ad, { 0xac, 0xa5, 0xa5, 0x49, 0x47, 0x5c, 0xf1, 0x85 } };
const GUID kIidMediaObjInfo =
{ 0x5137324d, 0x53c4, 0x4e2a, { 0xa8, 0x7f, 0x44, 0xc9, 0x61, 0x0a, 0x04, 0x55 } };

const DWORD kMediaInfoTypeMediaType = 0x0000;
const DWORD kMediaInfoTypeWidth = 0x0004;
const DWORD kMediaInfoTypeHeight = 0x0005;
const DWORD kMediaInfoTypeDuration = 0x0007;
const DWORD kMediaObjModeVideoRenderer = 5;
const DWORD kMediaObjModeGraph = 23;
const int kMediaObjRendererNull = 3;
const int kMediaObjGraphSnapshot = 1;

// Keep this prefix in exact ABI order with the PDR IMEDIAOBJ8 declaration.
// H2-02 needs only source loading and basic metadata; transport is H2-04.
struct IMediaObj8Source : IUnknown
{
    virtual void STDMETHODCALLTYPE SetEventCB(void* callback, void* caller) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetDisplayWnd(HWND parent, RECT rectangle) = 0;
    virtual HRESULT STDMETHODCALLTYPE LoadClip(LPSTR path, LONGLONG start, LONGLONG stop,
                                               BOOL alwaysRender) = 0;
    virtual HRESULT STDMETHODCALLTYPE LoadClip(LPWSTR path, LONGLONG start, LONGLONG stop,
                                               BOOL alwaysRender) = 0;
    virtual void STDMETHODCALLTYPE Unload() = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMediaInfo(DWORD type, LPVOID value) = 0;
};

struct IClRegPathSource : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE SetRegPath(HKEY* root, LPSTR path,
                                                 int pathLength, int subFolderType) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetRegPathW(HKEY* root, LPWSTR path,
                                                  int pathLength, int subFolderType) = 0;
};

struct IMediaObjInfoSource : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetModeInfo(DWORD mode, LPVOID value) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetModeInfo(DWORD mode, LPVOID value) = 0;
};

void ApplyPdrRegistryPath(IUnknown* mediaObj)
{
    IClRegPathSource* regPath = nullptr;
    if (FAILED(mediaObj->QueryInterface(kIidClRegPath,
        reinterpret_cast<void**>(&regPath))) || !regPath)
        return;

    HKEY root = HKEY_LOCAL_MACHINE;
    wchar_t path[] = L"Software\\CyberLink\\PowerDirector25";
    regPath->SetRegPathW(&root, path, static_cast<int>(std::size(path) - 1), 1);
    regPath->Release();
}

HRESULT ConfigureSourceOnlyGraph(IUnknown* mediaObj)
{
    IMediaObjInfoSource* mediaInfo = nullptr;
    const HRESULT queryResult = mediaObj->QueryInterface(kIidMediaObjInfo,
        reinterpret_cast<void**>(&mediaInfo));
    if (FAILED(queryResult) || !mediaInfo)
        return queryResult;

    int graphMode = kMediaObjGraphSnapshot;
    HRESULT result = mediaInfo->SetModeInfo(kMediaObjModeGraph, &graphMode);
    if (SUCCEEDED(result))
    {
        int renderer = kMediaObjRendererNull;
        result = mediaInfo->SetModeInfo(kMediaObjModeVideoRenderer, &renderer);
    }
    mediaInfo->Release();
    return result;
}

} // namespace

CMediaObjSourceAdapter::CMediaObjSourceAdapter() = default;

CMediaObjSourceAdapter::~CMediaObjSourceAdapter()
{
    Unload();
    if (m_comInitialized)
        ::CoUninitialize();
}

SourceMediaInfo CMediaObjSourceAdapter::Load(const std::string& utf8Path)
{
    Unload();
    if (utf8Path.empty())
    {
        m_info.statusText = "MediaObj source path is empty.";
        m_info.errorCode = E_INVALIDARG;
        return m_info;
    }

    const std::wstring path = ToWide(utf8Path);
    if (path.empty() || ::GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        m_info.statusText = "MediaObj source file is missing.";
        m_info.errorCode = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        return m_info;
    }

    if (!EnsureMediaObj())
        return m_info;

    std::vector<wchar_t> mutablePath(path.begin(), path.end());
    mutablePath.push_back(L'\0');
    IMediaObj8Source* mediaObj = static_cast<IMediaObj8Source*>(m_mediaObj);
    const HRESULT configureResult = ConfigureSourceOnlyGraph(mediaObj);
    if (FAILED(configureResult))
    {
        m_info.errorCode = configureResult;
        m_info.statusText = "MediaObj source graph setup failed: " + FormatHResult(configureResult);
        return m_info;
    }

    const HRESULT loadResult = mediaObj->LoadClip(mutablePath.data(), 0, 0, FALSE);
    if (FAILED(loadResult))
    {
        m_info.errorCode = loadResult;
        m_info.statusText = "MediaObj LoadClip failed: " + FormatHResult(loadResult);
        return m_info;
    }

    m_info.loaded = true;
    m_info.statusText = "MediaObj source loaded.";
    HRESULT result = mediaObj->GetMediaInfo(kMediaInfoTypeMediaType, &m_info.mediaType);
    if (FAILED(result))
    {
        m_info.errorCode = result;
        m_info.statusText = "MediaObj source loaded; media type unavailable: " + FormatHResult(result);
        return m_info;
    }

    const HRESULT widthResult = mediaObj->GetMediaInfo(kMediaInfoTypeWidth, &m_info.width);
    const HRESULT heightResult = mediaObj->GetMediaInfo(kMediaInfoTypeHeight, &m_info.height);
    m_info.hasDimensions = SUCCEEDED(widthResult) && SUCCEEDED(heightResult) &&
        m_info.width > 0 && m_info.height > 0;

    const HRESULT durationResult = mediaObj->GetMediaInfo(kMediaInfoTypeDuration, &m_info.duration100ns);
    m_info.hasDuration = SUCCEEDED(durationResult) && m_info.duration100ns > 0;
    return m_info;
}

void CMediaObjSourceAdapter::Unload()
{
    if (m_mediaObj)
    {
        IMediaObj8Source* mediaObj = static_cast<IMediaObj8Source*>(m_mediaObj);
        mediaObj->Unload();
        mediaObj->Release();
        m_mediaObj = nullptr;
    }
    m_info = SourceMediaInfo{};
}

bool CMediaObjSourceAdapter::EnsureMediaObj()
{
    if (m_mediaObj)
        return true;

    if (!m_comInitialized)
    {
        const HRESULT initializeResult = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (FAILED(initializeResult) && initializeResult != RPC_E_CHANGED_MODE)
        {
            m_info.errorCode = initializeResult;
            m_info.statusText = "MediaObj COM initialization failed: " + FormatHResult(initializeResult);
            return false;
        }
        m_comInitialized = initializeResult == S_OK || initializeResult == S_FALSE;
    }

    IMediaObj8Source* mediaObj = nullptr;
    const HRESULT createResult = ::CoCreateInstance(kClsidMediaObj, nullptr,
        CLSCTX_INPROC_SERVER, kIidMediaObj13, reinterpret_cast<void**>(&mediaObj));
    if (FAILED(createResult) || !mediaObj)
    {
        m_info.errorCode = createResult;
        m_info.statusText = "MediaObj activation failed: " + FormatHResult(createResult);
        return false;
    }

    ApplyPdrRegistryPath(mediaObj);
    m_mediaObj = mediaObj;
    return true;
}

std::wstring CMediaObjSourceAdapter::ToWide(const std::string& utf8Text)
{
    if (utf8Text.empty())
        return std::wstring();

    const int size = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
        utf8Text.c_str(), -1, nullptr, 0);
    if (size <= 1)
        return std::wstring();

    std::wstring result(static_cast<size_t>(size), L'\0');
    ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8Text.c_str(), -1,
        result.data(), size);
    result.pop_back();
    return result;
}

std::string CMediaObjSourceAdapter::FormatHResult(long result)
{
    std::ostringstream stream;
    stream << "0x" << std::hex << std::uppercase << static_cast<unsigned long>(result);
    return stream.str();
}
