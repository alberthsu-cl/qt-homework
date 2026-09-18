#pragma once
//
// QtKitHost - a ~150-line stand-in for PDR's QtKitWrapper.
//
// It does exactly what src/ui/QtKitWrapper/QtKitWrapper.cpp does, minus the
// DPI/theme/language/logging configuration that a demo does not need:
//
//   1. LoadLibraryEx("QtKit.dll") from the directory holding our .exe
//   2. GetProcAddress("createQtKit")  -> IQtKit*
//   3. push configuration while still single-threaded
//   4. run(async = true, onContextCreated, onContextRemoved)
//   5. everything after that happens on the Qt thread
//
// Read this side by side with QtKitWrapper.cpp. The shape is deliberately the
// same so the mapping is obvious.
//

#include <windows.h>
#include <functional>
#include <string>

#include "QtKit/Interface.h"

// QtKit calls back into the host to build engine-backed QML items (video
// preview sources, the on-screen editor, the font/colour pickers). PDR wires
// these to real delegates in QtKitWrapper.cpp; this demo needs none of them,
// but the factory itself is NOT optional - QtKit dereferences it while building
// the QML context, and passing nothing crashes at startup with 0xC0000005.
class QtKitHost : public ICreateFactory
{
public:
    static QtKitHost& Inst();

    // Appends one line to %TEMP%\QmlColorPiker.log. The demo has no console and
    // OutputDebugString needs a debugger attached, so this is how you find out
    // how far startup got.
    static void Log(const char* fmt, ...);

    // Loads QtKit.dll and starts the Qt thread. Returns false if the DLL is
    // missing or does not export createQtKit.
    //
    // onReady fires ON THE QT THREAD once IQmlContext exists. That is the
    // earliest moment anything QML-related may be touched - see the long
    // comment about qApp in QtKitWrapper.cpp.
    bool Start(std::function<void()> onReady);

    // Mirrors QtKitWrapper::~QtKitWrapper(): quit the runloop, then freeQtKit.
    void Stop();

    IQmlContext* Context() { return m_pContext; }
    bool IsLoaded() const { return m_hQtKit != nullptr; }

    // Directory the .exe lives in, with a trailing backslash, UTF-8.
    // QtKit.dll, the Qt6 DLLs and platforms\qwindows.dll are all expected here,
    // which is why the demo builds into its OWN bin_x64\ and stages a private
    // copy of the runtime there (stage_runtime.ps1). It deliberately does NOT
    // build into PDR's bin_x64\PowerDirector\: that folder is force-tracked in
    // the PDR repo, so output there shows up as a repo modification.
    const std::string& ExeDir() const { return m_strExeDir; }

    // Loads a .qml entry on the Qt thread and calls onLoaded once the WHOLE
    // object tree has been constructed.
    //
    // Use this, not setBindAction(), to decide when it is safe to reach for
    // your bindings. QML runs Component.onCompleted per object, so a single
    // id's bind action can fire long before its siblings exist - adopt on the
    // window's bind action and you will touch buttons that are not bound yet.
    // PDR takes the same route: QtKitWrapper::load(path, onComplete, onFail),
    // with CQtEntryViewController::OnQmlDidLoadImp hanging off onComplete.
    void LoadQml(const std::string& strPathUtf8, std::function<void()> onLoaded);
    void UnloadQml(const std::string& strPathUtf8);

protected:
    // ---- ICreateFactory: all null, this demo instantiates none of them ----
    std::shared_ptr<IMOSourceProtocol>        createMOSourceDelegate() override        { return nullptr; }
    std::shared_ptr<ISimbaSourceProtocol>     createSimbaSourceDelegate() override     { return nullptr; }
    std::shared_ptr<IOnScreenEditorProtocol>  createOnScreenEditorDelegate() override  { return nullptr; }
    std::shared_ptr<IFontPickerSourceProtocol>  createFontPickerSourceDelegate() override  { return nullptr; }
    std::shared_ptr<IColorPickerSourceProtocol> createColorPickerSourceDelegate() override { return nullptr; }

private:
    QtKitHost() = default;
    ~QtKitHost() = default;
    QtKitHost(const QtKitHost&) = delete;
    QtKitHost& operator=(const QtKitHost&) = delete;

    HMODULE      m_hQtKit{ nullptr };
    IQtKit*      m_pQtKit{ nullptr };
    IQmlContext* m_pContext{ nullptr };   // valid only between onReady and Stop()
    std::string  m_strExeDir;
};
