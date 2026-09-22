# timelineEditor - timeline editing (homework - 3)

A timeline editing surface that runs **inside PowerDirector**, not beside it.

The first two homeworks are standalone: they need no Qt SDK and no PDR
checkout. This one is different on purpose, and the reason is the whole point
of the exercise.

## Why this one is hosted

PowerDirector's timeline is the SIMBA engine. Homework-2 tried to drive SIMBA
from a standalone executable and measured the answer: construction faults with
`0xC0000005`, from two separate locations, with a `SIMBA.dll` byte-identical to
the product's. The binary was never missing.

SIMBA is not a self-contained COM object. It expects a CyberLink registry path,
CLRC feature config, a font-manager secret key, AC3 codec init, MUI resources,
and a message drain window with a running pump. Staging more DLLs does not
close that gap, because the missing pieces are product state and a host.

So Homework-3 boots PDR into its own command-line mode, reuses the headless
prelude that `clpdrMCP` and `clpdrEngineTest` already share, and puts a
QtKit/QML timeline in a visible host window on top of a real engine.

## What it teaches

Homework-1 and Homework-2 answered *how does MFC host QML*. This one answers a
harder question:

> How do you add a feature to a codebase whose engine access is a 12,000-line
> singleton with 6,897 call sites, without making the coupling worse?

The answer is the two layers the product already built to contain that
singleton -- the `ISimbaWrapper` test seam and the `ITimeline` abstraction with
its UI-free realizer. Homework-3 builds on those and touches `CSimbaWrapper`
in exactly one place: engine bring-up.

The deliverable is as much the boundary as the timeline.

## Scope

Add track, add clip, remove clips, ripple trim. A QML timeline rendering
tracks, clips, and a playhead from an engine snapshot. Composed preview
playback. Undo through the realizer's revert points.

No effects, transitions, titles, keyframes, persistence, or export -- and no
reuse of the 68,527-line MFC `WorkSpace` timeline this is meant to replace.

## Status

Planning. No code yet. The engineering record starts at
[`docs/README.md`](docs/README.md), and the survey that fixed every decision
in it is [`docs/SIMBA_INTEGRATION.md`](docs/SIMBA_INTEGRATION.md).

## Building

Requires a built parent PDR checkout; this project cannot be opened on its own.
Build and run instructions land with `H3-01`, once the host mode exists.
