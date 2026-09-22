# Media Player demo

A standalone MFC + QtKit/QML prototype for the first mini-editor milestone. It
uses the same runtime bridge as the other homework projects and does not need a
Qt SDK.

## Current milestone

The project demonstrates the complete UI and controller loop for:

- importing one media file;
- showing it in the media library;
- source play, pause, stop, and seek controls;
- a preview region for the selected source;
- an asset-information Properties panel;
- separated QML components for each editor region.

Image files render directly in the preview. Video and audio files show the
MediaObj source-player placeholder. SIMBA, timelines, editing commands, and
effects are outside the Homework-2 scope.

## QML layout

```text
qml/
|-- MediaPlayerWindow.qml
|-- MediaPlayerName.qml
|-- MediaPlayerProperty.qml
|-- MediaLibrary/
|   |-- MediaLibraryPanel.qml
|   `-- MediaTile.qml
|-- Preview/
|   |-- PreviewPanel.qml
|   `-- PlaybackControls.qml
`-- Properties/
    `-- ClipPropertiesPanel.qml
```

`MediaPlayerWindow.qml` composes the regions. The individual files own local
presentation behavior, while `CMediaPlayerController` owns imported-media and
source playback state. `CMediaPlayerViewController` is the only
C++ class that knows QML binding names or the Qt child window.

## Build and run

```bat
cd qt-homework\mediaPlayer
msbuild MediaPlayer.sln /p:Configuration=Debug /p:Platform=x64
bin_x64\MediaPlayer.exe
```

The post-build event stages two runtimes beside the executable:

1. `..\QtKit\runtime` supplies QtKit and Qt 6 runtime files.
2. `..\..\external_bin\runtime` supplies the curated SIMBA/MediaObj playback
   package through `stage_playback_runtime.ps1`.

No Qt headers, import libraries, `moc`, CMake Qt discovery, or installed Qt SDK
are used. The application includes only the pure virtual QtKit bridge header
from `..\QtKit\include` and loads `QtKit.dll` dynamically.

Each successful build also deploys `MediaPlayer.exe` beside `PDR.exe` in
`..\..\bin_x64\PowerDirector`, along with a private `MediaPlayerQml`
folder. It does not copy or overwrite PDR's QtKit or playback runtime files.
This PDR-hosted copy is useful when testing the executable against the product
runtime environment; the normal self-contained `bin_x64` build remains the
default development launch.

An optional media path can be supplied on the command line:

```bat
bin_x64\MediaPlayer.exe "C:\media\sample.jpg"
```

Startup and QML diagnostics are written to `%TEMP%\MediaPlayer.log`.
The editor does not activate proprietary engines during ordinary startup.
Run the opt-in diagnostic below when reviewing the staged playback runtime:

```powershell
.\bin_x64\MediaPlayer.exe --probe-playback-runtime
```

`ME-M0-01` records staged playback-runtime paths, their
architecture, registered COM server paths, and MediaObj/SIMBA activation
results. A copied DLL is not treated as activated until its COM class and
required interface have both been verified.

MediaObj and SIMBA use registration-free COM. `MediaPlayer.manifest` is
merged into the executable and maps the engine and supporting COM classes to
their staged DLL paths. Running `regsvr32` or installing machine-wide COM
entries is not required.

## Engineering plan

Implementation is tracked as an issue graph with architecture decisions and
explicit verification rules. Start with the active
[`docs/HOMEWORK_2_PLAN.md`](docs/HOMEWORK_2_PLAN.md). The original
[`docs/ISSUE_GRAPH.md`](docs/ISSUE_GRAPH.md) remains the deferred PDR
engine-integration backlog; proprietary playback is no longer a Homework-2
startup or delivery gate.
