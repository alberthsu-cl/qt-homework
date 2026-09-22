# Current Media Player architecture

## Scope

Homework-2 is a single-source MediaObj player. It imports one file, publishes
selected-asset state to QML, and will provide source preview and transport only
for formats that pass the MO-only runtime verification.

Timeline composition, editing, effects, persistence, and SIMBA are outside
this project.

## Ownership boundary

```text
MFC main frame
  -> MediaPlayerController: application state and user intent
  -> MediaObj source adapter: COM lifetime, LoadClip, metadata, errors
  -> MediaPlayerViewController: QtKit host and QML bindings
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

## Current implementation state

The QtKit/QML shell, MO-only runtime staging, and command-line MediaObj probes
exist. The next implementation item is the H2-02 source adapter; a passing
probe is runtime evidence, not yet application playback integration.
