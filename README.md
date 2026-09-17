# qt-homework

Onboarding notes for PowerDirector (PDR), written while getting a fresh clone
building and then working out how the Qt integration actually fits together.

Audience: an RD who is new to this codebase.

> **Scratch/personal notes — not part of the product tree.** Nothing here is
> referenced by the build, and none of it is indexed by `docs/`. If a document
> here proves generally useful, the right home is `docs/` under the relevant
> feature folder (see `docs/features/README.md` and the `docs-index-sync`
> workflow), not this folder.

---

## Contents

| File | What it covers |
|---|---|
| [`doc/BUILD_SETUP.md`](doc/BUILD_SETUP.md) | Why a fresh clone fails to link with `LNK1104: cannot open file 'CtrlFactoryD.lib'`, and the eModule step that fixes it. Includes the failure modes hit on a real first run. |
| [`doc/QTKIT_ARCHITECTURE.md`](doc/QTKIT_ARCHITECTURE.md) | How the MFC application hosts Qt 6 QML through `QtKit.dll` — module map, boot sequence, binding model, window embedding, threading rules, dual-mode switches, resource pipeline. Mermaid diagrams render in VS Code (the `bierner.markdown-mermaid` extension `bootstrap.bat` installs). |
| [`doc/QtKit-Bridge.html`](doc/QtKit-Bridge.html) | The same architecture brief as a self-contained HTML page with hand-drawn SVG diagrams. Open it directly in a browser. |

Plus two runnable demo projects, [described below](#demo-projects).

Published copy of the HTML brief (same content, shareable link):
https://claude.ai/code/artifact/7d568ad8-b78b-4a70-b468-ddd410f74bb0

---

## The 60-second version

**Qt is not linked into PowerDirector.** No Qt header, no Qt symbol, no `moc`
pass anywhere in this repo. The entire Qt world is sealed inside `QtKit.dll`,
loaded at runtime with `LoadLibraryEx` and reached through exactly two
`extern "C"` exports plus pure-virtual structs in
`src/external include/QtKit/Interface.h`.

Four consequences you will meet in week one:

1. **It runs on its own thread.** `qtKit->run(async=true)` returns *before* the
   QML context exists. Use `RunOnQtThreadAsync` / `RunOnMainThreadAsync`; never
   touch the other side directly.
2. **QML objects are addressed by string.** `context->button("dpPlayBtn")`.
   Rename an `objectName` and nothing fails to compile — it throws
   `QmlObjectNoBoundError` at runtime.
3. **Both UIs ship in every build.** A per-module registry DWORD under
   `HKCU\Software\CyberLink\PowerDirector25\QtMigration\` picks which is wired
   up. Timeline is still **OFF** — the MFC ruler is live and kept in sync.
4. **`src/ui/common/MigrateCandy.h` is the front door.** `Say()` / `Listen()` is
   the vocabulary every migrated module speaks.

---

## Two corrections to existing docs

Both verified against source at `e82e4c817e`, both still unfixed upstream:

- **`AGENTS.md` names the wrong registry root.** It documents
  `...\CyberLink\PowerDirector24\QtMigration\Library`. The live root is
  **`PowerDirector25`** (`STR_PDR_FULL_NAME`, `src/ui/definition/PDRPathDef.cpp:16`).
  `PowerDirector24` is now the first entry in `STR_PDR_FULL_NAME_PREV`, used for
  settings migration from older installs.

- **A `Debug|x64` build loads the *release* `QtKit.dll`.** `QtKitWrapper`
  branches on `#ifdef DEBUG`, but no project here defines bare `DEBUG` — MSVC
  uses `_DEBUG`, which is what all the `.vcxproj` files set. The branch is dead,
  so `QtKitd.dll` ships at 5.2 MB and is never opened through this path.

---

## Demo projects

Two standalone MFC applications, each in its own Visual Studio solution. Both
host Qt 6 QML through `QtKit.dll` the way PowerDirector does, and both are
deliberately kept separate so either can be opened without ambiguity.

Neither needs a Qt SDK. Each ships a `stage_runtime.ps1` that copies the
minimum QtKit + Qt6 runtime out of `bin_x64\PowerDirector\` into the demo's
own `bin_x64\` - after which the demo loads nothing from the product output.
Both do need a built PDR checkout two levels up, for that staging source and
for `src\external include\QtKit\Interface.h`.

| | [`mfc_qml_demo/`](mfc_qml_demo/) | [`color_picker_demo/`](color_picker_demo/) |
|---|---|---|
| **Question it answers** | How does MFC host QML at all? | What does a real feature built on that look like? |
| **The UI** | An image canvas with zoom / rotate icon buttons | A full-screen image viewer with an HSV color picker over it |
| **Size** | ~1,100 lines C++, ~400 lines QML | ~1,400 lines C++, ~900 lines QML |
| **Solution** | `MfcQmlDemo.sln` | `QmlColorPiker.sln` |

### `mfc_qml_demo/` - the bridge, minimally

The reference project. An MFC app loads `QtKit.dll` at runtime, starts Qt on a
second thread, adopts the `QQuickWindow`'s `HWND` into its own frame, and
talks across the boundary in both directions: **File > Open Image** pushes a
picture into QML, a QML button click lands in the native status strip, and
Zoom / Rotate make the full round trip - QML reports intent, `CDemoController`
decides, QML redraws from the result.

Split into controller / view-controller / QML the way a real PDR feature is,
so the layering is visible and not just described. Read its README for the
mapping onto `CQtController` / `CQtEntryViewController`, the 116-file runtime
inventory taken from the live process, and **eight failure modes that each
cost a build cycle** - every one of which presented as a silent no-op or an
access violation rather than an error message.

```bat
cd qt-homework\mfc_qml_demo
powershell -ExecutionPolicy Bypass -File .\stage_runtime.ps1
msbuild MfcQmlDemo.sln /p:Configuration=Debug /p:Platform=x64
bin_x64\MfcQmlDemo.exe
```

### `color_picker_demo/` - a feature on top of the bridge

Keeps the hosting shape above and replaces the transform controls with a
reusable HSV ColorPicker: hue/saturation board, value slider, hex and RGB
readout, preset swatches, and up to twelve custom colors persisted to
`custom_colors.txt`. **File > Open Preview Image** supplies a real image; the
Color button opens the picker as its own movable top-level Qt window beside
the host, pending edits tint the image live, Apply returns the final
`#RRGGBB` to C++ and Cancel restores the previous tint.

The interesting part is the boundary, not the widget. QtKit callbacks are
**not** used for high-frequency HSV edits - both property and button callbacks
can destabilize this runtime under rapid QML updates - so QML owns the
immediate preview and C++ reads `colorHex` only at Apply / Add-custom. The
tint is a Qt Quick overlay rather than a media effect, on purpose: it proves
out the controller / view-controller / QML / return-value contract before
anyone commits to the player route. See
[`COLOR_PICKER.md`](color_picker_demo/COLOR_PICKER.md) for that contract and
the extraction notes.

It is also the only demo here with **two** view controllers.
`CColorPickerViewController` owns the picker's QML entry on its own property
channel and is unloaded on Apply or Cancel, so tearing the picker down cannot
unbind the long-lived `DemoWindow` state - and unlike `CDemoViewController`,
its window is never reparented into the MFC frame, which is what keeps Qt's
own DPI sizing intact.

```bat
cd qt-homework\color_picker_demo
powershell -ExecutionPolicy Bypass -File .\stage_runtime.ps1 -Verify
msbuild QmlColorPiker.sln /p:Configuration=Debug /p:Platform=x64
bin_x64\QmlColorPiker.exe
```

Both executables also accept an image path, to skip the file dialog:

```bat
bin_x64\QmlColorPiker.exe "C:\pictures\example.png"
```

## Next areas to write up

Not started yet — listed so the gap is visible:

- **WorkSpace / Timeline** — the largest unmigrated module, and the one whose
  dual-maintenance burden the Qt migration is currently paying for.
- **Agentic Editing API** — `agentic_editing_harness/` submodule plus
  `docs/features/agentic-editing/`; has its own `add-agentic-tool` skill.
- **Engine boundary** — CES / Simba / the `IMOSourceProtocol` and
  `ISimbaSourceProtocol` delegates that feed preview textures into QML.
