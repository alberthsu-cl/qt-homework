# Current Media Player architecture

## Scope

Homework-2 is a single-source MediaObj player. It imports one file, publishes
selected-asset state to QML, and will provide source preview and transport only
for formats that pass the MediaObj source-runtime verification.

Timeline composition, editing, effects, persistence, and SIMBA are outside
this project.

## Ownership boundary

```text
MFC main frame
  -> MediaPlayerController: application state and user intent
  -> MediaObj source adapter: COM lifetime, source loading, transport, errors
  -> MediaPlayerViewController: QtKit host and QML bindings
  -> MFC frame: native MediaObj display parent and frame-local preview rect
  -> QML: Media Library, Preview, Properties, and transport presentation
```

Only the MediaObj source adapter may retain a proprietary COM interface. The
controller and QML exchange project-owned state and commands, never a
MediaObj pointer.

## Thread and lifetime contract

- Create, use, unload, and release MediaObj on its owning native thread.
- Unload the current source before replacement or application shutdown.
- Let engine results, not a QML timer, define source position and transport
  state.
- Keep QML binding and child-window lifetime inside `CMediaPlayerViewController`.
- Let QML report the preview rectangle in window coordinates; marshal its
  changes to the MFC thread before calling MediaObj.

## Current implementation state

The QtKit/QML shell, media catalog, MediaObj source adapter, runtime staging,
H2-03 native MediaObj preview placement and H2-04 transport calls exist, but
decoded video remains blocked: `SetVisible(TRUE)` returns `E_FAIL` for the
video-only graph with the staged MO runtime. The MFC playback timer samples
MediaObj's position only when playback is available. Seeking remains a later
milestone.
