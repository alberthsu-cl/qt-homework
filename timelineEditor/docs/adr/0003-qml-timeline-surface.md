# ADR-0003: The timeline UI uses the product's Qt module pattern

- Status: Accepted
- Date: 2026-09-22
- Revised: 2026-09-22 -- first draft pointed at the standalone demos' hosting
  style; corrected to the product's own stack. See "What the first draft got
  wrong" below.

## Decision

Homework-3's timeline is a Qt/QML module built the way PowerDirector builds
every new Qt module, under `src/ui/{module}/Qt{Module}/`:

```text
CQtModelessController          entry point from MFC (a window, not a modal dialog)
  CQtEntryViewController       owns the QML window and qmlContext; implements GetQmlPath()
    CQtViewController
      CQtStaticSubController / CQtDynamicSubController
```

with the product's lifecycle hooks -- `InitEntryViewController`,
`RegisterNotification`, `OnQmlDidLoad`, `OnQmlWillUnload` -- and `Present()` /
`Dismiss(result)`.

`src/ui/WorkSpace/` is not extended, subclassed, or reused.

## What the first draft got wrong

The first version of this ADR said Homework-3 would host QML "the same way
`mfc_qml_demo` and the media player do: QtKit loaded at runtime, Qt on its own
thread, the `QQuickWindow`'s `HWND` adopted into the MFC frame."

That is the right answer for a **standalone** demo and the wrong one here. The
first two homeworks hand-roll that sequence because they have no product
infrastructure to lean on. Homework-3 runs inside PowerDirector, where the
infrastructure is the point: `src/ui/QtKitWrapper/` already handles QML loading
and qmlContext binding, and `CQtEntryViewController` already owns the window.

Hand-rolling it inside the product would have meant a second, parallel QML
hosting style in the same process -- exactly the kind of duplication ADR-0002
refuses for the engine, applied to the UI.

## Evidence

- `Verified`: `QtKit.dll` is a product component (`bin_x64/PowerDirector/QtKit.dll`,
  wrapper source in `src/ui/QtKitWrapper/`). The homeworks stage it *from* the
  product; it is not a homework asset.
- `Verified`: the parent repo documents the layered pattern and its lifecycle
  hooks as the way new Qt modules are written
  (`docs/common/architecture.md`), with a step-by-step migration SOP in
  `docs/common/qt-migration.md`.
- `Verified`: the headless prelude already performs the resource, DPI, grafix,
  font, and UI-manager initialization a Qt/MFC surface needs -- it skips the
  launcher and main panel, not the UI infrastructure.
- `Verified`: `WorkSpace` is 68,527 lines of MFC timeline rendering.
- `Unknown`: whether a `CQtModelessController` presents correctly from a
  command-line host window that has no main panel behind it. H3-01 proves this
  or records what else the prelude must do first.
- `Unknown`: whether the preview canvas and the QML window cooperate in one
  frame, or whether preview needs its own child `HWND` region. H3-06 decides.

## Consequences

- Homework-3 follows `docs/common/qt-migration.md` and the QML conventions in
  `docs/common/qml-guidelines.md` (theme system, image naming, AutoTestId,
  binding pattern) rather than inventing homework-local conventions.
- The QML lives in the product's resource pipeline (`skinQt/`, `.qrc`/`.rcc`),
  not beside a homework executable.
- What carries over from the first two homeworks is **understanding**, not
  code: the boundary lessons, and which callback routes destabilize under rapid
  QML updates. No files are shared.
- Anything added to `common.dll` must be registered in **both**
  `common.vcxproj` and `common.vcxproj.filters`, or it silently does not exist.

## Verification

`VR-RENDER-01`, `VR-PREVIEW-01`.
