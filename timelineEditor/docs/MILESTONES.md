# Homework-3 milestones: product-hosted timeline editor

## Goal

A timeline editing surface running inside PowerDirector: a QtKit/QML timeline
that renders real SIMBA state and edits it through `ITimeline`, with preview
playback and revert-point undo.

## In scope

- a command-line host mode reusing the existing headless prelude and pump;
- SIMBA bring-up, movie binding, and ordered teardown;
- QML rendering of tracks, clips, and a playhead from a snapshot;
- add track, add clip, remove clips, ripple trim;
- composed timeline preview playback;
- revert-point undo, failure paths, clean shutdown.

## Out of scope

- effects, transitions, titles, subtitles, stickers, keyframes;
- project persistence and produce/export;
- 360, motion tracking, and every other designer;
- any reuse of `src/ui/WorkSpace/`;
- a standalone executable.

## Milestones

| # | Milestone | Tickets | Done when |
|---|---|---|---|
| M1 | A live engine in our own host | H3-01, H3-02 | The mode boots, SIMBA reports ready, the pump runs, and a failed bring-up still yields a usable shell. |
| M2 | A timeline on screen | H3-03, H3-04 | A bound movie's snapshot renders as tracks and clips addressed by stable id. |
| M3 | Editing that reaches the engine | H3-05 | All four commands change engine state, confirmed by product read-back; failures surface in the UI. |
| M4 | Preview and undo | H3-06, H3-07 | Playback follows engine position; a revert point restores the pre-command timeline; twenty cycles leave no surviving process. |

## Exit criteria

- H3-01 through H3-07 complete, each with its verification rules evidenced;
- ADR-0003 records the preview-surface decision with runtime evidence;
- the three open questions in [adr/README.md](adr/README.md) are answered or
  explicitly carried forward with an owner;
- `VR-COUPLING-01` holds: no new `CSimbaWrapper::Inst()` call site outside the
  host bring-up;
- the parent solution still builds `Debug|x64` with the mode present and unused.

## Dependency on the parent checkout

Unlike Homework-1 and Homework-2, this project cannot be built or run on its
own. It requires a built PDR checkout, and its sources live in the product
tree. This directory is the engineering record; the code is a product change.
