#include "pch.h"
#include "DemoViewController.h"
#include "DemoController.h"
#include "MainFrame.h"      // WM_APP_QML_CLICK and the QmlClick codes
#include "QtKitHost.h"
#include "DemoNames.h"

CDemoViewController::CDemoViewController(CDemoController* pController, HWND hNotify)
    : m_pController(pController)
    , m_hNotify(hNotify)
{
}

CDemoViewController::~CDemoViewController()
{
    Dismiss();
}

bool CDemoViewController::Present()
{
    // The callback lands on the Qt thread.
    return QtKitHost::Inst().Start([this]() { OnQtReady(); });
}

void CDemoViewController::Dismiss()
{
    // Detach the Qt child before the parent HWND goes away, then unload.
    // PDR does the equivalent in CQtEntryViewController::OnQmlWillUnloadImp.
    if (m_hQmlWnd && ::IsWindow(m_hQmlWnd))
    {
        ::SetParent(m_hQmlWnd, nullptr);
        m_hQmlWnd = nullptr;
    }
    if (!m_strQmlEntry.empty())
    {
        QtKitHost::Inst().UnloadQml(m_strQmlEntry);
        m_strQmlEntry.clear();
    }
    m_bQmlReady = false;
}

// =============================================================================
//  QT THREAD from here down (OnQtReady, OnQmlDidLoad, and every lambda)
// =============================================================================

void CDemoViewController::OnQtReady()
{
    if (!QtKitHost::Inst().Context())
        return;

    // The .qml is read straight off disk (no skinQt.rcc here), so the entry is
    // a filesystem path relative to the .exe. The .exe lives in
    // <demo>\bin_x64\ and the QML in <demo>\qml\, hence one level up.
    //
    // Shipping builds take the other branch of that fork: PDR packs its QML
    // into skinQt.rcc and loads ":/qml/..." instead. See QTKIT_ARCHITECTURE.md
    // section 07 - every CQtEntryViewController is constructed with both paths
    // and picks between them on isRccFileReady().
    m_strQmlEntry = QtKitHost::Inst().ExeDir() + "../qml/DemoWindow.qml";

    // Adopt only once the entire tree is built. Hanging this off
    // setBindAction(window) instead looks equivalent and is not: the window's
    // bind action fires while objects further down the tree are still unbound.
    QtKitHost::Inst().LoadQml(m_strQmlEntry, [this]() { OnQmlDidLoad(); });
}

void CDemoViewController::OnQmlDidLoad()
{
    IQmlContext* pContext = QtKitHost::Inst().Context();
    if (!pContext)
        return;

    QtKitHost::Log("bound? window=%d property=%d caption=%d photo=%d open=%d add=%d",
                   (int)pContext->isObjectBound(DemoName.window),
                   (int)pContext->isObjectBound(DemoName.property),
                   (int)pContext->isObjectBound(DemoName.caption),
                   (int)pContext->isObjectBound(DemoName.photo),
                   (int)pContext->isObjectBound(DemoName.openPickerButton),
                   (int)pContext->isObjectBound(DemoName.addCustomButton),
                   (int)pContext->isObjectBound(DemoName.resetButton));

    if (!pContext->isObjectBound(DemoName.window))
        return;

    // ---- 1. pull the native handle out of Qt ------------------------------
    HWND hQml = nullptr;
    try
    {
        hQml = static_cast<HWND>(pContext->window(DemoName.window).window());
    }
    catch (const QmlObjectNoBoundError& e)
    {
        QtKitHost::Log("window not bound: %s", e.what());
        return;
    }
    if (!hQml)
        return;

    QtKitHost::Log("got HWND %p", (void*)hQml);
    m_hQmlWnd = hQml;

    // ---- 2. reparent it under the MFC frame -------------------------------
    // This is PDR's CQtEntryViewController::FillInParentWnd() in two lines.
    // After SetParent the QQuickWindow is an ordinary Win32 child: no
    // compositing layer, no shared surface, no Qt widget wrapper.
    // Reparent THROUGH QtKit, not with a raw ::SetParent. IUIWindow::setParent
    // and setPosition go through Qt, so the QML scene actually reflows to the
    // new size. A bare ::SetWindowPos moves the native window and leaves the
    // scene at its declared logical size, and anchored content then renders
    // off the right and bottom edges - which looks exactly like a DPI bug and
    // is not one.
    pContext->window(DemoName.window).setParent(m_hNotify);

    // Strip every top-level decoration. Qt.FramelessWindowHint in the .qml
    // covers most of it; these two lines make sure nothing survives to eat
    // client area or paint a second caption inside our frame.
    ::SetWindowLongPtr(hQml, GWL_STYLE,
        (::GetWindowLongPtr(hQml, GWL_STYLE) | WS_CHILD)
        & ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX));
    ::SetWindowLongPtr(hQml, GWL_EXSTYLE,
        ::GetWindowLongPtr(hQml, GWL_EXSTYLE) & ~(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME));
    ::SetWindowPos(hQml, nullptr, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    QtKitHost::Log("reparented under %p", (void*)m_hNotify);

    // ---- 3. wire QML -> C++ ------------------------------------------------
    QtKitHost::Log("wiring picker controls");
    WireButtons(pContext);

    // ---- 4. seed the shared state block ------------------------------------
    // QML starts with the defaults declared in DemoProperty.qml; this makes the
    // C++ side authoritative from the first frame rather than from the first
    // click. PDR seeds the same way, from OnQmlDidLoad.
    QtKitHost::Log("publishing picker state");
    m_pController->PublishState();
    QtKitHost::Log("picker state published");

    m_bQmlReady = true;
    QtKitHost::Log("bindings wired - demo ready");

    // Do NOT size the Qt child from here. We are on the Qt thread, moments
    // after load(), and Qt has not finished realising the window - the
    // SetWindowPos lands but the QML scene keeps its declared 1000x700 logical
    // size, so anything anchored to the window bottom renders off-screen.
    // Hand the job to the MFC thread instead, which is where window layout
    // belongs anyway.
    ::PostMessage(m_hNotify, WM_APP_QML_CLICK, 0, 0);   // 0 = "ready"
}

void CDemoViewController::WireButtons(IQmlContext* pContext)
{
    // ALWAYS check isObjectBound() first. The accessors do not return null for
    // an unknown name: QtKit logs "The id (x) does not exist" and hands back a
    // reference you then fault on. This is the C++ side of the loose,
    // string-keyed boundary, and it is why PDR's controllers are littered with
    // CQtViewController::IsQmlBound() calls.
    //
    // Note what each lambda does NOT do: it does not decide anything. The QML
    // button reports that it was pressed; the controller owns what that means.
    // Before this layer existed, QML clamped the zoom itself and C++ only
    // counted clicks.
    using Action = void (CDemoController::*)();
    struct { const char* name; QmlClick code; Action fn; } kButtons[] = {
        { DemoName.openPickerButton, kClickOpenPicker, &CDemoController::BeginColorEdit },
        { DemoName.resetButton,      kClickReset,      &CDemoController::ResetPendingColor },
        { DemoName.cancelButton,     kClickCancel,     &CDemoController::CancelColorEdit },
    };

    for (const auto& b : kButtons)
    {
        if (!pContext->isObjectBound(b.name))
        {
            QtKitHost::Log("button %s NOT bound - skipped", b.name);
            continue;
        }
        const QmlClick code = b.code;
        const Action   fn   = b.fn;
        pContext->button(b.name).setClickedAction([this, code, fn]() {
            // Already on the Qt thread, which is where IUIProperty writes have
            // to happen - so the controller can publish without hopping.
            (m_pController->*fn)();
            // The MFC status strip is a different thread's business.
            ::PostMessage(m_hNotify, WM_APP_QML_CLICK, static_cast<WPARAM>(code), 0);
        });
    }

    // QtKit is unstable when QML calls a C++ button callback for every HSV
    // mouse move. The preview remains entirely in QML while editing. These
    // two low-frequency actions take one property snapshot at user intent
    // boundaries, so the controller receives only saved or applied values.
    const auto wireColorSnapshot = [this, pContext](const char* name, QmlClick code, bool apply) {
        if (!pContext->isObjectBound(name) || !pContext->isObjectBound(DemoName.property))
            return;
        pContext->button(name).setClickedAction([this, pContext, code, apply]() {
            m_pController->SetPendingColorHex(pContext->property(DemoName.property).propertyString("colorHex"));
            if (apply)
                m_pController->ApplyColorEdit();
            else
                m_pController->AddCustomColor();
            ::PostMessage(m_hNotify, WM_APP_QML_CLICK, static_cast<WPARAM>(code), 0);
        });
    };
    wireColorSnapshot(DemoName.addCustomButton, kClickAddCustom, false);
    wireColorSnapshot(DemoName.applyButton, kClickApply, true);
}

// =============================================================================
//  callable from the MFC thread - each hops to the Qt thread itself
// =============================================================================

void CDemoViewController::SetViewportSize(int cx, int cy)
{
    IQmlContext* pContext = QtKitHost::Inst().Context();
    if (!m_bQmlReady || !pContext || !m_hQmlWnd || !::IsWindow(m_hQmlWnd))
        return;

    QtKitHost::Log("SetViewportSize -> setPosition(0,0,%d,%d)", cx, cy);
    pContext->runOnQtThread([pContext, cx, cy]() {
        if (pContext->isObjectBound(DemoName.window))
            pContext->window(DemoName.window).setPosition(0, 0, cx, cy);
    });
}

void CDemoViewController::PushImage(const std::string& strUrlUtf8,
                                    const std::string& strCaptionUtf8)
{
    IQmlContext* pContext = QtKitHost::Inst().Context();
    if (!pContext || !m_bQmlReady)
        return;

    // We are on the MFC thread here, so hop to the Qt thread before touching
    // any IUI* proxy. This is the mirror of the PostMessage going the other way.
    //
    // These two stay out of DemoProperty: a caption and an image URL are
    // view output, not state the controller reasons about. Keeping them here
    // also leaves all five binding types visible in one demo.
    pContext->runOnQtThread([pContext, strUrlUtf8, strCaptionUtf8]() {
        if (pContext->isObjectBound(DemoName.photo))
            pContext->image(DemoName.photo).source(strUrlUtf8.c_str());
        if (pContext->isObjectBound(DemoName.caption))
            pContext->label(DemoName.caption).text(strCaptionUtf8.c_str());
        QtKitHost::Log("pushed image: %s", strUrlUtf8.c_str());
    });
}
