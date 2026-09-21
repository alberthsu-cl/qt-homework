# Milestones

## M0 - Architecture and runtime proof

Goal: prove that the curated proprietary runtime can be activated, exercised,
and shut down safely from the standalone application.

In scope:

- deterministic runtime discovery and diagnostics;
- COM activation of MediaObj and SIMBA;
- MediaObj metadata extraction;
- comparison of native-window and callback-based preview transport;
- repeatable shutdown without hangs or leaked engine processes;
- final adapter interfaces for M1.

Exit criteria:

- `ME-M0-01` through `ME-M0-05` are complete;
- ADR-0003 is accepted or rejected with recorded evidence;
- all M0 verification rules pass on `Debug|x64`;
- unsupported or license-dependent behavior produces an actionable error.

## M1 - Source media preview

Goal: import one supported media file and provide real source preview through
MediaObj.

In scope:

- file validation and metadata;
- source duration and stream capability display;
- play, pause, stop, and seek;
- preview resizing and visibility changes;
- audio-only and still-image behavior;
- engine errors surfaced in the status area.

Exit criteria:

- one representative video, audio file, and image pass the M1 verification
  matrix;
- repeated import and replacement do not retain the prior graph;
- closing during load, play, pause, and seek exits within the shutdown limit.

## M2 - Timeline playback

Goal: build a one-track SIMBA composition from the native timeline model and
play it in the preview.

Exit criteria:

- the timeline is the single source of truth;
- one clip can be added, played, stopped, and seeked through SIMBA;
- the QML playhead follows authoritative engine time;
- end-of-stream and shutdown are deterministic.

## M3 - Editing operations

Goal: support useful single-track editing without introducing a second model.

Exit criteria:

- split, trim, reorder, and ripple-delete commands are model operations;
- invalid edits are rejected without corrupting state;
- undo and redo cover every M3 command;
- playback rebuilds only when required by the accepted architecture.

## M4 - Properties and simple effects

Goal: connect the existing property controls to real clip parameters and a
small verified effect set.

Exit criteria:

- scale, position, opacity, fade, and selected simple effects round-trip
  between model, UI, and engine;
- unsupported effects are hidden or disabled;
- parameter boundary and reset tests pass.

## M5 - Persistence and hardening

Goal: make the milestone project repeatable, diagnosable, and portable inside
its supported environment.

Exit criteria:

- project save/load round-trips the supported model;
- missing media is reported and can be relinked;
- runtime packaging is verified from a clean clone;
- smoke, shutdown, and failure-path checks are scripted and documented.

