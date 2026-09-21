# ADR-0003: Preview transport

- Status: Proposed
- Date: 2026-09-21

## Context

MediaObj exposes both a native display-window route and output callbacks. The
mini editor embeds a Qt Quick window inside an MFC frame, so either route has
integration risks that cannot be settled from headers alone.

## Options

### A - Native child `HWND`

Let MediaObj/SIMBA render into a child preview window positioned over or beside
the QML preview region.

Expected strengths: closest to established PDR behavior and no per-frame CPU
copy. Risks: sibling z-order, clipping, DPI, focus, and QML overlay behavior.

### B - Frame output callback

Receive engine frames and expose them to a QtKit/QML video source or image
provider.

Expected strengths: natural QML composition and overlays. Risks: proprietary
buffer lifetime, GPU/CPU synchronization, format conversion, backpressure, and
shutdown while callbacks are active.

## Decision rule

`ME-M0-03` selects the lowest-complexity route that passes resize, seek,
clipping, DPI, and shutdown verification without putting proprietary headers in
QtKit. The rejected option and a fallback trigger must be retained here before
this ADR becomes Accepted.

## Verification

`VR-PREVIEW-01`, `VR-LIFE-01`, `VR-THREAD-01`.

