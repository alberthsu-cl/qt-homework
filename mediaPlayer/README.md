# Media Player demo

A standalone MFC + QtKit/QML prototype for the first mini-editor milestone. It
uses the same runtime bridge as the other homework projects and does not need a
Qt SDK.

## Current milestone

The project demonstrates the complete UI and controller loop for:

- importing one media file;
- showing it in the media library;
- source play, pause, stop, and a disabled seek-control placeholder;
- a preview region for the selected source;
- an asset-information Properties panel;
- separated QML components for each editor region.

Image files render directly in the preview. Video playback is not yet working
in the MO-only runtime: source metadata loads, but the video-only graph fails
at `SetVisible(TRUE)` with `E_FAIL` for the sample MP4s. The UI now reports
that failure and disables transport instead of presenting a blank surface as
successful playback. SIMBA, timelines, editing commands, and effects are
outside the Homework-2 scope.

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
2. `..\..\external_bin\runtime` supplies the MediaObj source package through
   `stage_mediaobj_runtime.ps1`, including the verified MP4 splitter and H.264
   decoder files. It does not stage SIMBA.

No Qt headers, import libraries, `moc`, CMake Qt discovery, or installed Qt SDK
are used. The application includes only the pure virtual QtKit bridge header
from `..\QtKit\include` and loads `QtKit.dll` dynamically.

The normal self-contained `bin_x64` build is the default development launch.
`stage_pdr_hosted_demo.ps1` remains available as an explicit product-runtime
comparison tool, but it is not run by the build.

An optional media path can be supplied on the command line:

```bat
bin_x64\MediaPlayer.exe "C:\media\sample.jpg"
```

Startup and QML diagnostics are written to `%TEMP%\MediaPlayer.log`.
The editor does not activate proprietary engines during ordinary startup.
Run the opt-in activation diagnostic below when reviewing the staged MediaObj
source runtime:

```powershell
.\bin_x64\MediaPlayer.exe --probe-playback-runtime
```

`ME-M0-01` records staged MO runtime paths, their architecture, registered
COM server paths, and MediaObj activation results. A copied DLL is not treated
as activated until its COM class and required interface have both been
verified. To probe an actual source without starting the UI:

```powershell
.\bin_x64\MediaPlayer.exe --probe-mediaobj "C:\media\sample.mp3"
.\bin_x64\MediaPlayer.exe --probe-mediaobj-preview "C:\media\sample.mp4"
.\bin_x64\MediaPlayer.exe --probe-mediaobj-transport "C:\media\sample.mp4"
```

The preview probe creates a private native window and verifies source loading
with preview configuration without opening the full QML application. It does
not verify decoded video frames.
The transport probe additionally verifies play, position, pause, and stop
through the same MediaObj adapter used by the application.

The current bundle opens the supplied MP3 and H.264 MP4 samples and reads their
metadata. Images continue to use the direct QML image path rather than a
MediaObj playback graph.

MediaObj uses registration-free COM. `MediaPlayer.manifest` is merged into the
executable and maps the engine and supporting MO classes to their staged DLL
paths. Running `regsvr32` or installing machine-wide COM entries is not
required.

## Engineering plan

Implementation is tracked as an issue graph with architecture decisions and
explicit verification rules. Start with the active
[`docs/HOMEWORK_2_PLAN.md`](docs/HOMEWORK_2_PLAN.md). The original
[`docs/ISSUE_GRAPH.md`](docs/ISSUE_GRAPH.md) remains the deferred PDR
engine-integration backlog; proprietary playback is no longer a Homework-2
startup or delivery gate.
