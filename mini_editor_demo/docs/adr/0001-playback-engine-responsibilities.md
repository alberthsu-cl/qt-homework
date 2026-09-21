# ADR-0001: Playback engine responsibilities

- Status: Accepted
- Date: 2026-09-21

## Decision

MediaObj owns imported-source inspection and source preview. SIMBA owns composed
timeline playback. Project-owned adapters hide both proprietary APIs from the
controller and QML layers.

## Evidence

- Verified: `MediaObjWrapper` exposes load/unload, media information, source
  transport, seeking, display-window, snapshot, and output-callback operations.
- Verified: PDR's display panel uses MediaObj for source display and
  `CSimbaWrapper` for timeline composition/playback.
- Inferred: keeping these roles avoids building a SIMBA graph merely to inspect
  one source clip.
- Unknown: the minimum redistributable interface subset for this standalone
  homework executable remains subject to M0 activation tests and licensing.

## Consequences

- Source mode and timeline mode have separate adapters and explicit switching.
- The controller receives normalized metadata, time, events, and errors.
- QML never includes or names MediaObj or SIMBA interfaces.
- M1 can complete before the SIMBA timeline builder exists.

## Verification

`VR-RUNTIME-01`, `VR-RUNTIME-02`, `VR-MEDIA-01`, `VR-LIFE-01`.

