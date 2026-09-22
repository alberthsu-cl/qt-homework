# Homework-2 milestone: MediaObj source player

## Goal

Deliver a small MFC + QtKit/QML source-media player. A user imports one asset,
sees its information, and controls real MediaObj source playback.

## In scope

- media-library import and selected-asset state;
- MediaObj load, unload, metadata, and error normalization;
- one preview surface for the selected source;
- play, pause, stop, and seek;
- read-only asset information in Properties;
- invalid, missing, audio-only, image, replacement, and shutdown behavior.

## Out of scope

- timelines and multi-clip composition;
- split, trim, ripple, undo, and project persistence;
- transforms, fades, effects, and effect rendering;
- SIMBA activation or any SIMBA dependency.

## Exit criteria

- H2-01 through H2-06 are complete;
- ADR-0003 selects and records the preview route;
- the MediaObj verification matrix passes in `Debug|x64`;
- the editor starts even when the proprietary runtime is unavailable and gives
  an actionable diagnostic.
