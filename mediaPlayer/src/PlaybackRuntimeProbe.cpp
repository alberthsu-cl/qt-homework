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

// These identifiers are copied from the checked-in PDR MediaGUIDs.h and
// SIMBAGUID.h. The probe intentionally uses only IUnknown so proprietary
// engine headers do not become a dependency of this standalone project.
const GUID kClsidMediaObj =
{ 0x302147f3, 0x0a17, 0x4fff, { 0x89, 0x78, 0xf8, 0xb7, 0xa6, 0x4f, 0xcd, 0xa7 } };
const GUID kIidMediaObj13 =
{ 0x35eb2c0c, 0x373f, 0x4abb, { 0xaa, 0xe1, 0x62, 0x25, 0xcb, 0x35, 0x8e, 0xcc } };
const GUID kClsidSimba =
{ 0x9cfc585c, 0xd304, 0x4702, { 0x81, 0x61, 0x28, 0xc7, 0x94, 0x78, 0xda, 0x1d } };
const GUID kIidSimbaMovieEdit3 =
{ 0xf9572f02, 0x644a, 0x4495, { 0x8f, 0x0e, 0x96, 0x4e, 0x1f, 0x7a, 0x52, 0x11 } };

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
    if (result.path.size() >= 4 && result.path.substr(result.path.size() - 4) == L".dll")
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

bool ProbeComClass(const wchar_t* label,
                   const GUID& clsid,
                   const GUID& requiredInterface,
                   bool& comInitialized)
{
    const HRESULT comResult = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    comInitialized = SUCCEEDED(comResult);
    const std::wstring comText = FormatHResult(comResult);
    QtKitHost::Log("[M0-01] %ls CoInitializeEx(MTA): %s (%ls)", label,
                   comInitialized ? "ok" : "FAILED", comText.c_str());
    if (!comInitialized)
        return false;

    const std::wstring registeredServer = RegisteredServerPath(clsid);
    QtKitHost::Log("[M0-01] %ls registry server: %ls", label,
                   registeredServer.empty() ? L"<not registered>" : registeredServer.c_str());

    IUnknown* object = nullptr;
    const HRESULT activateResult = ::CoCreateInstance(
        clsid, nullptr, CLSCTX_INPROC_SERVER, IID_IUnknown,
        reinterpret_cast<void**>(&object));
    if (FAILED(activateResult) || !object)
    {
        const std::wstring text = FormatHResult(activateResult);
        QtKitHost::Log("[M0-01] %ls activation FAILED: %ls", label, text.c_str());
        ::CoUninitialize();
        return false;
    }

    void* required = nullptr;
    const HRESULT queryResult = object->QueryInterface(requiredInterface, &required);
    if (required)
        reinterpret_cast<IUnknown*>(required)->Release();
    object->Release();

    const std::wstring text = FormatHResult(queryResult);
    QtKitHost::Log("[M0-01] %ls activation ok; required interface %ls: %s (%ls)",
                   label, GuidText(requiredInterface).c_str(),
                   SUCCEEDED(queryResult) ? "ok" : "FAILED", text.c_str());

    ULONG_PTR contextToken = 0;
    const HRESULT contextResult = ::CoGetContextToken(&contextToken);
    if (SUCCEEDED(contextResult))
    {
        ::CoUninitialize();
    }
    else
    {
        QtKitHost::Log("[M0-01] %ls release removed the probe COM scope: %ls",
                       label, FormatHResult(contextResult).c_str());
    }
    return SUCCEEDED(queryResult);
}

bool ProbeSimbaOutOfProcess()
{
    wchar_t executablePath[MAX_PATH] = { 0 };
    ::GetModuleFileNameW(nullptr, executablePath, MAX_PATH);
    const std::wstring executable = executablePath;
    const size_t separator = executable.find_last_of(L"\\/");
    const std::wstring workingDirectory = separator == std::wstring::npos
        ? std::wstring() : executable.substr(0, separator);
    std::wstring commandLine = L"\"" + executable + L"\" --probe-simba-child";
    std::vector<wchar_t> mutableCommand(commandLine.begin(), commandLine.end());
    mutableCommand.push_back(L'\0');

    STARTUPINFOW startup = { 0 };
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process = { 0 };
    if (!::CreateProcessW(executable.c_str(), mutableCommand.data(), nullptr, nullptr,
                          FALSE, CREATE_NO_WINDOW, nullptr, workingDirectory.c_str(),
                          &startup, &process))
    {
        QtKitHost::Log("[M0-01] SIMBA child probe failed to start (win32=%lu)",
                       ::GetLastError());
        return false;
    }

    const DWORD waitResult = ::WaitForSingleObject(process.hProcess, 30000);
    DWORD exitCode = ERROR_TIMEOUT;
    if (waitResult == WAIT_OBJECT_0)
    {
        ::GetExitCodeProcess(process.hProcess, &exitCode);
    }
    else
    {
        ::TerminateProcess(process.hProcess, ERROR_TIMEOUT);
        ::WaitForSingleObject(process.hProcess, 5000);
    }

    ::CloseHandle(process.hThread);
    ::CloseHandle(process.hProcess);
    QtKitHost::Log("[M0-01] SIMBA child probe exit: 0x%08lX", exitCode);
    return exitCode == ERROR_SUCCESS;
}

} // namespace

PlaybackRuntimeProbeResult CPlaybackRuntimeProbe::Run(const std::wstring& executableDirectory)
{
    PlaybackRuntimeProbeResult result;
    QtKitHost::Log("[M0-01] ==== playback runtime activation probe ====");

    const FileProbe simba = ProbeFile(executableDirectory, L"runtime\\simba\\SIMBA.dll", L"SIMBA.dll");
    const FileProbe mediaObj = ProbeFile(executableDirectory, L"runtime\\mediacache\\MediaObj.dll", L"MediaObj.dll");
    const FileProbe mediaObjExt = ProbeFile(executableDirectory, L"runtime\\mediacache\\MediaObjExt.dll", L"MediaObjExt.dll");
    const FileProbe mediaObjIni = ProbeFile(executableDirectory, L"runtime\\mediacache\\MediaObj.ini", L"MediaObj.ini");

    result.runtimeFilesPresent = simba.present && simba.is64Bit && mediaObj.present &&
        mediaObj.is64Bit && mediaObjExt.present && mediaObjExt.is64Bit && mediaObjIni.present;
    if (!result.runtimeFilesPresent)
    {
        result.statusText = L"Playback runtime unavailable - staged files are missing or not x64. See MediaPlayer.log.";
        QtKitHost::Log("[M0-01] RESULT: staged runtime incomplete");
        return result;
    }

    bool mediaObjComInitialized = false;
    result.mediaObjActivated = ProbeComClass(
        L"MediaObj", kClsidMediaObj, kIidMediaObj13, mediaObjComInitialized);
    result.simbaActivated = ProbeSimbaOutOfProcess();
    result.comInitialized = mediaObjComInitialized;

    result.statusText = result.IsReady()
        ? L"Playback runtime ready - M0 activation probe passed."
        : L"Playback runtime unavailable - activation details are in MediaPlayer.log.";
    QtKitHost::Log("[M0-01] RESULT: %s", result.IsReady() ? "pass" : "FAILED");
    return result;
}

int CPlaybackRuntimeProbe::RunSimbaChild()
{
    bool comInitialized = false;
    const bool activated = ProbeComClass(
        L"SIMBA-child", kClsidSimba, kIidSimbaMovieEdit3, comInitialized);
    return comInitialized && activated ? ERROR_SUCCESS : ERROR_OPEN_FAILED;
}
