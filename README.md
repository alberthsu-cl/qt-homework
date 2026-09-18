# qt-homework

## Demo projects

Two standalone MFC applications, each in its own Visual Studio solution. Both
host Qt 6 QML through `QtKit.dll` the way PowerDirector does, and both are
deliberately kept separate so either can be opened without ambiguity.

Neither needs a Qt SDK or a PowerDirector checkout. The repository tracks the
shared QtKit interface and runtime under `QtKit/`; each project stages that
runtime into its own `bin_x64/` directory automatically after a build.

| | [`mfc_qml_demo/`](mfc_qml_demo/) | [`color_picker_demo/`](color_picker_demo/) |
|---|---|---|
| **Question it answers** | How does MFC host QML at all? | What does a real feature built on that look like? |
| **The UI** | An image canvas with zoom / rotate icon buttons | A full-screen image viewer, plus an HSV color picker in its own window |
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
msbuild MfcQmlDemo.sln /p:Configuration=Debug /p:Platform=x64
bin_x64\MfcQmlDemo.exe
```

### `color_picker_demo/` - a feature on top of the bridge (homework - 1)

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
msbuild QmlColorPiker.sln /p:Configuration=Debug /p:Platform=x64
bin_x64\QmlColorPiker.exe
```

Both builds stage `QtKit/runtime` automatically. Each project also keeps a
`stage_runtime.ps1` wrapper for manual `-Clean` or `-Verify` runs.

Both executables also accept an image path, to skip the file dialog:

```bat
bin_x64\QmlColorPiker.exe "C:\pictures\example.png"
```

## Documents

| File | What it covers |
|---|---|
| [`doc/QTKIT_ARCHITECTURE.md`](doc/QTKIT_ARCHITECTURE.md) | How the MFC application hosts Qt 6 QML through `QtKit.dll` — module map, boot sequence, binding model, window embedding, threading rules, dual-mode switches, resource pipeline. Mermaid diagrams render in VS Code (the `bierner.markdown-mermaid` extension `bootstrap.bat` installs). |
| [`doc/QtKit-Bridge.html`](doc/QtKit-Bridge.html) | The same architecture brief as a self-contained HTML page with hand-drawn SVG diagrams. Open it directly in a browser. |
