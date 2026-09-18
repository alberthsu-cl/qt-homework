# QmlColorPiker

`QmlColorPiker` is a standalone QtKit ColorPicker homework. It is intentionally
separate from the reference image demo in `../mfc_qml_demo` so both projects
can be opened in Visual Studio without ambiguity.

It keeps the useful MFC plus QtKit hosting shape from the reference project,
but replaces its image-transform controls with a reusable HSV ColorPicker. The
native MFC `File > Open Preview Image` command supplies a real image to QML.
The viewer stays full-screen; its Color button opens the picker over the image.

## What it exercises

```text
MFC File menu
    -> CMainFrame::PushImageToQml(file:///...)
    -> CDemoViewController::PushImage()
    -> QtKit UIImage binding
    -> QML Image + live tint overlay

HSV board / hue slider / presets
    -> DemoProperty.colorHex
    -> pending preview tint
    -> Apply snapshots the final hex into C++
    -> controller-owned #RRGGBB return value
```

QtKit callbacks are not used for high-frequency HSV edits: both property and
button callbacks can destabilize this runtime during rapid QML updates. While
the picker is open, QML owns the immediate tint preview. Apply or Add custom
snapshots the final `colorHex` into C++; Cancel restores the prior applied tint.

## Build and run

```bat
cd qt-homework\color_picker_demo
msbuild QmlColorPiker.sln /t:Build /p:Configuration=Debug /p:Platform=x64
bin_x64\QmlColorPiker.exe
```

The build automatically copies the committed runtime from `../QtKit/runtime`
into `bin_x64`. To stage or verify it manually, the original command remains
available:

```powershell
powershell -ExecutionPolicy Bypass -File .\stage_runtime.ps1 -Verify
```

You can also supply an image directly:

```bat
bin_x64\QmlColorPiker.exe "C:\pictures\example.png"
```

The project requires Visual Studio 2022 with the C++ and MFC components, but no
Qt SDK and no PowerDirector checkout. The QtKit interface and complete runtime
are stored in the repository's `QtKit` folder.

## PDR migration map

| Homework piece | PDR replacement |
| --- | --- |
| `CDemoController` | feature controller/model owning the selected color |
| `CDemoViewController` | `CQtEntryViewController` or modal controller |
| `qml/DemoWindow.qml` | `skinQt/qml/<Feature>/<Feature>.qml` |
| `DemoProperty.qml` | `<Feature>Property.qml` |
| MFC `PushImageToQml` | PDR preview/player bridge |

The current image tint is purposely a Qt Quick overlay rather than a media
effect. This validates the ColorPicker's controller, view-controller, QML,
and return-color contract before choosing the later media-player route.

See [COLOR_PICKER.md](COLOR_PICKER.md) for the control-level contract and
portable extraction notes.
