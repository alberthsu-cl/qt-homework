# Mini Editor demo

A standalone MFC + QtKit/QML prototype for the first mini-editor milestone. It
uses the same runtime bridge as the other homework projects and does not need a
Qt SDK.

## Current milestone

The project demonstrates the complete UI and controller loop for:

- importing one media file;
- showing it in the media library;
- adding the selected item to a single video timeline;
- play, pause, stop, and animated playhead state;
- adding split markers;
- basic scale, opacity, and effect controls;
- separated QML components for each editor region.

Image files render directly in the preview. Video and audio files show the
engine render target placeholder. The build stages the SIMBA, MediaObj, DSP,
and CES playback binaries, but the native frame-rendering adapter is the next
integration step; this project does not claim decoded video playback yet.

## QML layout

```text
qml/
|-- MiniEditorWindow.qml
|-- MiniEditorName.qml
|-- MiniEditorProperty.qml
|-- MediaLibrary/
|   |-- MediaLibraryPanel.qml
|   `-- MediaTile.qml
|-- Preview/
|   |-- PreviewPanel.qml
|   `-- PlaybackControls.qml
|-- Timeline/
|   |-- TimelinePanel.qml
|   |-- TimelineTrack.qml
|   `-- TimelineClip.qml
`-- Properties/
    |-- ClipPropertiesPanel.qml
    `-- EffectControls.qml
```

`MiniEditorWindow.qml` composes the regions. The individual files own local
presentation behavior, while `CMiniEditorController` owns imported-media,
timeline, playback, and split state. `CMiniEditorViewController` is the only
C++ class that knows QML binding names or the Qt child window.

## Build and run

```bat
cd qt-homework\mini_editor_demo
msbuild MiniEditorDemo.sln /p:Configuration=Debug /p:Platform=x64
bin_x64\MiniEditorDemo.exe
```

The post-build event stages two runtimes beside the executable:

1. `..\QtKit\runtime` supplies QtKit and Qt 6 runtime files.
2. `..\..\external_bin\runtime` supplies the curated SIMBA/MediaObj playback
   package through `stage_playback_runtime.ps1`.

No Qt headers, import libraries, `moc`, CMake Qt discovery, or installed Qt SDK
are used. The application includes only the pure virtual QtKit bridge header
from `..\QtKit\include` and loads `QtKit.dll` dynamically.

An optional media path can be supplied on the command line:

```bat
bin_x64\MiniEditorDemo.exe "C:\media\sample.jpg"
```

Startup and QML diagnostics are written to `%TEMP%\MiniEditorDemo.log`.

## Next engine step

Implement a playback adapter behind the controller:

- MediaObj for metadata and source-clip preview;
- SIMBA for timeline composition and synchronized playback;
- a QtKit video source delegate for frames rendered into `PreviewPanel.qml`.

The QML component boundaries and controller commands are already arranged so
that this work does not require redesigning the UI.

## Engineering plan

Implementation is tracked as an issue graph with architecture decisions and
explicit verification rules. Start at [`docs/README.md`](docs/README.md), then
use [`docs/ISSUE_GRAPH.md`](docs/ISSUE_GRAPH.md) for ticket order. M0 proves the
proprietary runtime and preview transport before M1 connects real source-media
playback to the finished layout.
