#include "pch.h"
#include "QtKitHost.h"

#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <shellscalingapi.h>
#pragma comment(lib, "Shcore.lib")

// The two C exports are the entire outbound surface of QtKit.dll.
// Signatures copied verbatim from QtKitWrapper.cpp.
typedef IQtKit* (__stdcall* FuncCreateQtKit)();
typedef void    (__stdcall* FuncFreeQtKit)(IQtKit*);

#include <cstdarg>
#include <cstdio>

void QtKitHost::Log(const char* fmt, ...)
{
    char szDir[MAX_PATH] = { 0 };
    ::GetTempPathA(MAX_PATH, szDir);
    std::string strPath = std::string(szDir) + "QmlColorPiker.log";

    FILE* fp = nullptr;
    if (fopen_s(&fp, strPath.c_str(), "a") != 0 || !fp)
        return;

    SYSTEMTIME st;
    ::GetLocalTime(&st);
    fprintf(fp, "[%02d:%02d:%02d.%03d][tid %5lu] ",
            st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, ::GetCurrentThreadId());

    va_list ap;
    va_start(ap, fmt);
    vfprintf(fp, fmt, ap);
    va_end(ap);

    fprintf(fp, "\n");
    fclose(fp);
}

namespace {

std::string ToUtf8(const wchar_t* wsz)
{
    if (!wsz || !*wsz) return std::string();
    int n = ::WideCharToMultiByte(CP_UTF8, 0, wsz, -1, nullptr, 0, nullptr, nullptr);
    if (n <= 1) return std::string();
    std::string s(static_cast<size_t>(n - 1), '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, wsz, -1, &s[0], n, nullptr, nullptr);
    return s;
}

} // namespace

QtKitHost& QtKitHost::Inst()
{
    static QtKitHost s_inst;
    return s_inst;
}

bool QtKitHost::Start(std::function<void()> onReady)
{
    if (m_hQtKit)
        return true;

    Log("---- Start() ----");

    // ---- 1. resolve QtKit.dll beside our own .exe --------------------------
    // Same GetModuleFileName + PathRemoveFileSpec dance as QtKitWrapper's ctor.
    wchar_t szPath[MAX_PATH] = { 0 };
    ::GetModuleFileNameW(nullptr, szPath, MAX_PATH);
    ::PathRemoveFileSpecW(szPath);
    ::PathAddBackslashW(szPath);
    m_strExeDir = ToUtf8(szPath);

    // NOTE: PDR picks QtKitd.dll under "#ifdef DEBUG" - a macro no .vcxproj in
    // this repo ever defines, so that branch is dead and even Debug|x64 loads
    // the release QtKit.dll. We do the same thing on purpose, so the demo and
    // the product load the identical binary.
    ::PathAppendW(szPath, L"QtKit.dll");

    // LOAD_WITH_ALTERED_SEARCH_PATH makes the DLL's own directory the first
    // place Windows looks for its dependencies (Qt6Core.dll and friends).
    m_hQtKit = ::LoadLibraryExW(szPath, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!m_hQtKit)
    {
        ::OutputDebugStringW(L"[demo] QtKit.dll failed to load\n");
        return false;
    }

    // ---- 2. the one and only entry point ----------------------------------
    FuncCreateQtKit createQtKit =
        reinterpret_cast<FuncCreateQtKit>(::GetProcAddress(m_hQtKit, "createQtKit"));
    if (!createQtKit)
    {
        ::OutputDebugStringW(L"[demo] createQtKit not exported\n");
        return false;
    }

    m_pQtKit = createQtKit();
    Log("createQtKit -> %p", (void*)m_pQtKit);
    if (!m_pQtKit)
        return false;

    // ---- 3. configure while still single-threaded ---------------------------
    // Everything here must happen BEFORE run(). QtKit reads these values when
    // it builds its QApplication.
    //
    // This block is not a menu of optional niceties. QtKit dereferences several
    // of these while constructing the QML context, so leaving one out is an
    // access violation at startup, not a degraded feature. The set below is the
    // minimum that actually starts; compare QtKitWrapper.cpp, which additionally
    // configures DPI, language and the .rcc.
    m_pQtKit->setAppName("QmlColorPiker");
    Log("setAppName ok");

    // The create-factory is REQUIRED even when every delegate is null.
    m_pQtKit->setCreateFactory(this);
    Log("setCreateFactory ok");

    // QtKit writes QSettings through these two paths. Unset = crash.
    char szTemp[MAX_PATH] = { 0 };
    ::GetTempPathA(MAX_PATH, szTemp);
    const std::string strUserIni = std::string(szTemp) + "QmlColorPiker_UserConfig.ini";
    const std::string strDsaIni  = std::string(szTemp) + "QmlColorPiker_DontShowAgain.ini";
    m_pQtKit->setUserConfigIniPath(strUserIni.c_str());
    m_pQtKit->setDontShowAgainIniPath(strDsaIni.c_str());
    Log("ini paths ok: %s", strUserIni.c_str());

    // No skinQt.rcc in this demo, so QtKit reads .qml straight off disk and
    // resolves "image://Media/..." against this directory. Harmless if unused.
    const std::string strMediaDir = m_strExeDir + "../../skinQt/media";
    m_pQtKit->setImageDir("Media", strMediaDir.c_str());

    // ---- DPI: the one number that decides whether the scene fits ----------
    // Qt is per-monitor DPI aware. This MFC host is not (no dpiAwareness in a
    // manifest), so Windows hands it virtualised coordinates: ask for a
    // 1000x680 frame on a 125% display and you get an 800x544 physical window.
    // Qt, meanwhile, renders the QML scene at the monitor's real scale. Leave
    // this at 1.0 and the scene is 25% too big for the window it lives in -
    // anchored content simply falls off the right and bottom edges.
    //
    // QtKitWrapper.cpp computes exactly this ratio; the only difference is that
    // PDR reads its app DPI from CDpiMgr, while an unaware host is always 96.
    // DemoApp.cpp calls SetProcessDpiAwarenessContext(PER_MONITOR_AWARE_V2)
    // before any window exists, so this host and Qt already measure in the
    // same physical pixels and the ratio is 1:1.
    //
    // PDR cannot use 1.0 here: CDpiMgr may hold an app DPI that differs from
    // the monitor the main window currently sits on, so QtKitWrapper.cpp
    // computes dpiApp/dpiDisplay per monitor. The general rule is the same
    // either way - this number must be (host DPI / monitor DPI), and getting
    // it wrong shows up as a QML scene that does not fit its own window.
    UINT dpiDisplay = 96;
    if (HMONITOR hMon = ::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY))
    {
        UINT dpiX = 96, dpiY = 96;
        if (SUCCEEDED(::GetDpiForMonitor(hMon, MDT_EFFECTIVE_DPI, &dpiX, &dpiY)))
            dpiDisplay = dpiY;
    }
    m_pQtKit->setDpiScale(1.0f);
    Log("dpi: monitor=%u, host is PER_MONITOR_AWARE_V2 -> setDpiScale(1.0)", dpiDisplay);
    m_pQtKit->setAppIconPath("PDR.ico");
    m_pQtKit->enableTheme(QmlThemeMode::Dark);
    Log("theme/dpi/icon ok");

    // Route QtKit's internal logging into our log file - this is where QML
    // syntax errors and missing-import messages surface.
    m_pQtKit->setDebugOutput([](const char* msg) {
        if (!msg || !*msg) return;
        QtKitHost::Log("[QtKit] %s", msg);
    });

    // ---- 4. start the Qt thread -------------------------------------------
    // async = true, so this returns immediately and the QApplication is built
    // on a thread QtKit owns. m_pContext is null until onReady fires.
    m_pQtKit->run(
        true,
        [this, onReady]() {
            // >>> we are on the Qt thread from here <<<
            m_pContext = m_pQtKit->context();
            Log("context created -> %p", (void*)m_pContext);
            if (onReady)
                onReady();
        },
        [this]() {
            // context is being torn down
            m_pContext = nullptr;
        });

    Log("run() returned, ctor thread continues");
    return true;
}

void QtKitHost::Stop()
{
    if (!m_pQtKit)
        return;

    m_pQtKit->quit();

    if (m_hQtKit)
    {
        FuncFreeQtKit freeQtKit =
            reinterpret_cast<FuncFreeQtKit>(::GetProcAddress(m_hQtKit, "freeQtKit"));
        if (freeQtKit)
            freeQtKit(m_pQtKit);
    }

    m_pQtKit = nullptr;
    m_pContext = nullptr;
    // Deliberately NOT FreeLibrary(m_hQtKit): Qt does not reliably survive
    // being unloaded from a live process, and PDR does not unload it either.
    m_hQtKit = nullptr;
}

void QtKitHost::LoadQml(const std::string& strPathUtf8, std::function<void()> onLoaded)
{
    if (!m_pContext)
        return;

    // Already on the Qt thread when called from onReady, but runOnQtThread is
    // safe either way - it runs inline if you are already there.
    m_pContext->runOnQtThread([this, strPathUtf8, onLoaded]() {
        bool bLoaded = false;
        try
        {
            Log("load(%s)", strPathUtf8.c_str());
            bLoaded = m_pContext->load(strPathUtf8.c_str(), QmlThemeMode::Dark);
            Log("IQmlContext::load -> %s", bLoaded ? "ok" : "FALSE");
        }
        catch (const std::string& err)
        {
            Log("load THREW: %s", err.c_str());
            return;
        }

        // load() returns only after the whole tree is built, so every
        // Component.onCompleted has run and every id is bound by now.
        if (bLoaded && onLoaded)
            onLoaded();
    });
}

void QtKitHost::UnloadQml(const std::string& strPathUtf8)
{
    if (!m_pContext)
        return;

    m_pContext->runOnQtThread([this, strPathUtf8]() {
        m_pContext->unload(strPathUtf8.c_str());
    });
}
