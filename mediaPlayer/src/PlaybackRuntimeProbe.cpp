#include "pch.h"
#include "PlaybackRuntimeProbe.h"
#include "QtKitHost.h"

#include <objbase.h>
#include <iterator>
#include <sstream>
#include <vector>

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Advapi32.lib")

namespace {

// This identifier is copied from the checked-in PDR MediaGUIDs.h. The narrow
// interface below is intentionally self-contained so this homework does not
// take a compile-time dependency on PDR's MediaObj headers.
const GUID kClsidMediaObj =
{ 0x302147f3, 0x0a17, 0x4fff, { 0x89, 0x78, 0xf8, 0xb7, 0xa6, 0x4f, 0xcd, 0xa7 } };
const GUID kIidMediaObj8 =
{ 0x201f0102, 0x23bd, 0x4ddd, { 0xa0, 0x44, 0xb0, 0x2b, 0x71, 0x0a, 0xc2, 0x79 } };
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
const UINT kMediaTypeAudio = 0x03;

// The first IMEDIAOBJ8 methods, copied verbatim in ABI order. We need only
// LoadClip, GetMediaInfo, and Unload for the source-open verification.
struct IMediaObj8Probe : IUnknown
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

struct IClRegPathProbe : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE SetRegPath(HKEY* root, LPSTR path,
                                                 int pathLength, int subFolderType) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetRegPathW(HKEY* root, LPWSTR path,
                                                  int pathLength, int subFolderType) = 0;
};

struct IMediaObjInfoProbe : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetModeInfo(DWORD mode, LPVOID value) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetModeInfo(DWORD mode, LPVOID value) = 0;
};

std::wstring FormatHResult(HRESULT hr);

void ApplyPdrRegistryPath(IUnknown* mediaObj)
{
    IClRegPathProbe* regPath = nullptr;
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
    IMediaObjInfoProbe* mediaInfo = nullptr;
    const HRESULT queryResult = mediaObj->QueryInterface(kIidMediaObjInfo,
        reinterpret_cast<void**>(&mediaInfo));
    if (FAILED(queryResult) || !mediaInfo)
        return queryResult;

    int graphMode = kMediaObjGraphSnapshot;
    const HRESULT graphResult = mediaInfo->SetModeInfo(kMediaObjModeGraph, &graphMode);
    int renderer = kMediaObjRendererNull;
    const HRESULT rendererResult = SUCCEEDED(graphResult)
        ? mediaInfo->SetModeInfo(kMediaObjModeVideoRenderer, &renderer)
        : graphResult;
    mediaInfo->Release();

    QtKitHost::Log("[M0-02] source graph setup: snapshot=%ls, null-renderer=%ls",
        FormatHResult(graphResult).c_str(), FormatHResult(rendererResult).c_str());
    return rendererResult;
}

struct FileProbe
{
    std::wstring label;
    std::wstring path;
    bool present{ false };
    bool hasArchitecture{ false };
    bool is64Bit{ false };
    DWORD error{ ERROR_SUCCESS };
};

std::wstring FormatHResult(HRESULT hr)
{
    wchar_t buffer[512] = { 0 };
    const DWORD count = ::FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, static_cast<DWORD>(hr), 0, buffer,
        static_cast<DWORD>(std::size(buffer)), nullptr);

    std::wstringstream stream;
    stream << L"0x" << std::hex << std::uppercase << static_cast<unsigned long>(hr);
    if (count > 0)
    {
        std::wstring message(buffer, count);
        while (!message.empty() && (message.back() == L'\r' || message.back() == L'\n'))
            message.pop_back();
        stream << L" (" << message << L")";
    }
    return stream.str();
}

std::wstring GuidText(const GUID& guid)
{
    wchar_t buffer[64] = { 0 };
    ::StringFromGUID2(guid, buffer, static_cast<int>(std::size(buffer)));
    return buffer;
}

bool Is64BitPortableExecutable(const std::wstring& path, DWORD& error)
{
    HANDLE file = ::CreateFileW(path.c_str(), GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        error = ::GetLastError();
        return false;
    }

    IMAGE_DOS_HEADER dosHeader = { 0 };
    DWORD bytesRead = 0;
    bool valid = ::ReadFile(file, &dosHeader, sizeof(dosHeader), &bytesRead, nullptr) &&
        bytesRead == sizeof(dosHeader) && dosHeader.e_magic == IMAGE_DOS_SIGNATURE;

    LARGE_INTEGER offset = { 0 };
    if (valid)
    {
        offset.QuadPart = dosHeader.e_lfanew;
        valid = ::SetFilePointerEx(file, offset, nullptr, FILE_BEGIN) != 0;
    }

    DWORD signature = 0;
    IMAGE_FILE_HEADER fileHeader = { 0 };
    if (valid)
    {
        valid = ::ReadFile(file, &signature, sizeof(signature), &bytesRead, nullptr) &&
            bytesRead == sizeof(signature) && signature == IMAGE_NT_SIGNATURE;
    }
    if (valid)
    {
        valid = ::ReadFile(file, &fileHeader, sizeof(fileHeader), &bytesRead, nullptr) &&
            bytesRead == sizeof(fileHeader);
    }
    ::CloseHandle(file);

    if (!valid)
    {
        error = ERROR_BAD_EXE_FORMAT;
        return false;
    }

    if (fileHeader.Machine != IMAGE_FILE_MACHINE_AMD64)
    {
        error = ERROR_BAD_EXE_FORMAT;
        return false;
    }

    error = ERROR_SUCCESS;
    return true;
}

FileProbe ProbeFile(const std::wstring& executableDirectory,
                    const wchar_t* relativePath,
                    const wchar_t* label)
{
    FileProbe result;
    result.label = label;
    result.path = executableDirectory + relativePath;

    const DWORD attributes = ::GetFileAttributesW(result.path.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
    {
        result.error = ::GetLastError();
        QtKitHost::Log("[M0-01] %ls missing: %ls (win32=%lu)",
                       result.label.c_str(), result.path.c_str(), result.error);
        return result;
    }

    result.present = true;
    const bool isDll = result.path.size() >= 4 &&
        result.path.substr(result.path.size() - 4) == L".dll";
    const bool isAx = result.path.size() >= 3 &&
        result.path.substr(result.path.size() - 3) == L".ax";
    if (isDll || isAx)
    {
        result.hasArchitecture = true;
        result.is64Bit = Is64BitPortableExecutable(result.path, result.error);
    }

    if (result.hasArchitecture)
    {
        QtKitHost::Log("[M0-01] %ls present: %ls (x64=%s, win32=%lu)",
                       result.label.c_str(), result.path.c_str(),
                       result.is64Bit ? "yes" : "no", result.error);
    }
    else
    {
        QtKitHost::Log("[M0-01] %ls present: %ls (architecture=n/a, win32=%lu)",
                       result.label.c_str(), result.path.c_str(), result.error);
    }
    return result;
}

std::wstring RegisteredServerPath(const GUID& clsid)
{
    const std::wstring key = L"CLSID\\" + GuidText(clsid) + L"\\InprocServer32";
    DWORD bytes = 0;
    const LONG sizeResult = ::RegGetValueW(HKEY_CLASSES_ROOT, key.c_str(), nullptr,
                                            RRF_RT_REG_SZ, nullptr, nullptr, &bytes);
    if (sizeResult != ERROR_SUCCESS || bytes == 0)
        return std::wstring();

    std::vector<wchar_t> value(bytes / sizeof(wchar_t), L'\0');
    const LONG readResult = ::RegGetValueW(HKEY_CLASSES_ROOT, key.c_str(), nullptr,
                                            RRF_RT_REG_SZ, nullptr, value.data(), &bytes);
    if (readResult != ERROR_SUCCESS)
        return std::wstring();
    return value.data();
}

bool ProbeMediaObjActivation(bool& comInitialized)
{
    const HRESULT comResult = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    comInitialized = SUCCEEDED(comResult);
    const std::wstring comText = FormatHResult(comResult);
    QtKitHost::Log("[M0-01] MediaObj CoInitializeEx(MTA): %s (%ls)",
                   comInitialized ? "ok" : "FAILED", comText.c_str());
    if (!comInitialized)
        return false;

    const std::wstring registeredServer = RegisteredServerPath(kClsidMediaObj);
    QtKitHost::Log("[M0-01] MediaObj registry server: %ls",
                   registeredServer.empty() ? L"<not registered>" : registeredServer.c_str());

    IUnknown* object = nullptr;
    const HRESULT activateResult = ::CoCreateInstance(
        kClsidMediaObj, nullptr, CLSCTX_INPROC_SERVER, IID_IUnknown,
        reinterpret_cast<void**>(&object));
    if (FAILED(activateResult) || !object)
    {
        const std::wstring text = FormatHResult(activateResult);
        QtKitHost::Log("[M0-01] MediaObj activation FAILED: %ls", text.c_str());
        ::CoUninitialize();
        return false;
    }

    void* required = nullptr;
    const HRESULT queryResult = object->QueryInterface(kIidMediaObj8, &required);
    if (required)
        reinterpret_cast<IUnknown*>(required)->Release();
    object->Release();

    const std::wstring text = FormatHResult(queryResult);
    QtKitHost::Log("[M0-01] MediaObj activation ok; IMEDIAOBJ8 %s (%ls)",
                   SUCCEEDED(queryResult) ? "ok" : "FAILED", text.c_str());

    ULONG_PTR contextToken = 0;
    const HRESULT contextResult = ::CoGetContextToken(&contextToken);
    if (SUCCEEDED(contextResult))
    {
        ::CoUninitialize();
    }
    else
    {
        QtKitHost::Log("[M0-01] MediaObj release removed the probe COM scope: %ls",
                       FormatHResult(contextResult).c_str());
    }
    return SUCCEEDED(queryResult);
}

} // namespace

PlaybackRuntimeProbeResult CPlaybackRuntimeProbe::Run(const std::wstring& executableDirectory)
{
    PlaybackRuntimeProbeResult result;
    QtKitHost::Log("[M0-01] ==== MediaObj source runtime activation probe ====");

    const FileProbe mediaObj = ProbeFile(executableDirectory, L"runtime\\mediacache\\MediaObj.dll", L"MediaObj.dll");
    const FileProbe mediaObjExt = ProbeFile(executableDirectory, L"runtime\\mediacache\\MediaObjExt.dll", L"MediaObjExt.dll");
    const FileProbe mediaObjIni = ProbeFile(executableDirectory, L"runtime\\mediacache\\MediaObj.ini", L"MediaObj.ini");
    const FileProbe mp4Splitter = ProbeFile(executableDirectory,
        L"runtime\\decoderPack\\CLM4Splt.ax", L"CLM4Splt.ax");
    const FileProbe videoDecoder = ProbeFile(executableDirectory,
        L"runtime\\decoderPack\\CLCVD\\clcvd.ax", L"clcvd.ax");
    const FileProbe h264Decoder = ProbeFile(executableDirectory,
        L"runtime\\decoderPack\\CLCVD\\264dsse2.dll", L"264dsse2.dll");
    const FileProbe decoderThreads = ProbeFile(executableDirectory,
        L"runtime\\decoderPack\\CLCVD\\pthreadVC2.dll", L"pthreadVC2.dll");
    const FileProbe decoderShell = ProbeFile(executableDirectory,
        L"runtime\\decoderPack\\CLCVD\\vdshell.dll", L"vdshell.dll");

    result.runtimeFilesPresent = mediaObj.present && mediaObj.is64Bit &&
        mediaObjExt.present && mediaObjExt.is64Bit && mediaObjIni.present &&
        mp4Splitter.present && mp4Splitter.is64Bit &&
        videoDecoder.present && videoDecoder.is64Bit &&
        h264Decoder.present && h264Decoder.is64Bit &&
        decoderThreads.present && decoderThreads.is64Bit &&
        decoderShell.present && decoderShell.is64Bit;
    if (!result.runtimeFilesPresent)
    {
        result.statusText = L"MediaObj source runtime unavailable - staged files are missing or not x64. See MediaPlayer.log.";
        QtKitHost::Log("[M0-01] RESULT: MediaObj source runtime incomplete");
        return result;
    }

    bool mediaObjComInitialized = false;
    result.mediaObjActivated = ProbeMediaObjActivation(mediaObjComInitialized);
    result.comInitialized = mediaObjComInitialized;

    result.statusText = result.IsReady()
        ? L"MediaObj source runtime ready - activation probe passed."
        : L"MediaObj source runtime unavailable - activation details are in MediaPlayer.log.";
    QtKitHost::Log("[M0-01] RESULT: %s", result.IsReady() ? "pass" : "FAILED");
    return result;
}

PlaybackRuntimeProbeResult CPlaybackRuntimeProbe::RunSourceProbe(
    const std::wstring& executableDirectory,
    const std::wstring& sourcePath)
{
    PlaybackRuntimeProbeResult result = Run(executableDirectory);
    if (!result.IsReady())
        return result;

    if (sourcePath.empty() || ::GetFileAttributesW(sourcePath.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        QtKitHost::Log("[M0-02] source missing: %ls", sourcePath.c_str());
        result.statusText = L"MediaObj source probe failed: source file is missing.";
        return result;
    }

    const HRESULT comResult = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(comResult))
    {
        QtKitHost::Log("[M0-02] CoInitializeEx failed: %ls", FormatHResult(comResult).c_str());
        result.statusText = L"MediaObj source probe failed: COM initialization failed.";
        return result;
    }

    IMediaObj8Probe* mediaObj = nullptr;
    const HRESULT createResult = ::CoCreateInstance(
        kClsidMediaObj, nullptr, CLSCTX_INPROC_SERVER, kIidMediaObj13,
        reinterpret_cast<void**>(&mediaObj));
    if (FAILED(createResult) || !mediaObj)
    {
        QtKitHost::Log("[M0-02] MediaObj creation failed: %ls", FormatHResult(createResult).c_str());
        ::CoUninitialize();
        result.statusText = L"MediaObj source probe failed: MediaObj creation failed.";
        return result;
    }

    ApplyPdrRegistryPath(mediaObj);

    const HRESULT configureResult = ConfigureSourceOnlyGraph(mediaObj);
    if (FAILED(configureResult))
    {
        QtKitHost::Log("[M0-02] source graph setup failed: %ls",
            FormatHResult(configureResult).c_str());
        mediaObj->Release();
        ::CoUninitialize();
        result.statusText = L"MediaObj source probe failed: source graph setup failed.";
        return result;
    }

    std::vector<wchar_t> mutablePath(sourcePath.begin(), sourcePath.end());
    mutablePath.push_back(L'\0');
    const HRESULT loadResult = mediaObj->LoadClip(mutablePath.data(), 0, 0, FALSE);
    QtKitHost::Log("[M0-02] LoadClip(%ls): %ls", sourcePath.c_str(), FormatHResult(loadResult).c_str());
    if (SUCCEEDED(loadResult))
    {
        UINT mediaType = 0;
        UINT width = 0;
        UINT height = 0;
        LONGLONG duration = 0;
        const HRESULT mediaTypeResult = mediaObj->GetMediaInfo(kMediaInfoTypeMediaType, &mediaType);
        const HRESULT widthResult = mediaObj->GetMediaInfo(kMediaInfoTypeWidth, &width);
        const HRESULT heightResult = mediaObj->GetMediaInfo(kMediaInfoTypeHeight, &height);
        const HRESULT durationResult = mediaObj->GetMediaInfo(kMediaInfoTypeDuration, &duration);
        QtKitHost::Log("[M0-02] metadata: type=%u (%ls), size=%ux%u (%ls, %ls), duration100ns=%lld (%ls)",
            mediaType, FormatHResult(mediaTypeResult).c_str(), width, height,
            FormatHResult(widthResult).c_str(), FormatHResult(heightResult).c_str(),
            duration, FormatHResult(durationResult).c_str());
        const bool hasDimensions = SUCCEEDED(widthResult) && SUCCEEDED(heightResult);
        const bool hasDuration = SUCCEEDED(durationResult) && duration > 0;
        result.sourceOpened = SUCCEEDED(mediaTypeResult) &&
            (mediaType == kMediaTypeAudio ? hasDuration : hasDimensions);
    }

    mediaObj->Unload();
    mediaObj->Release();
    ::CoUninitialize();
    result.statusText = result.sourceOpened
        ? L"MediaObj source probe passed."
        : L"MediaObj source probe failed. See MediaPlayer.log.";
    QtKitHost::Log("[M0-02] RESULT: %s", result.sourceOpened ? "pass" : "FAILED");
    return result;
}
