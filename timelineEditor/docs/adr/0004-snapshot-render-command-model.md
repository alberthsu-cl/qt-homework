# ADR-0004: Snapshot rendering, command intent, revert-point undo

- Status: Accepted
- Date: 2026-09-22

## Decision

QML renders `ITimeline::snapshot()` and emits **intent**. The controller turns
one intent into one `ITimeline` call, then re-reads the snapshot and re-renders.
Clips and tracks are addressed by the stable ids the snapshot carries, never by
QML list index.

Undo is the realizer's revert point (`armRevertPoint` / `revertToPoint` /
`commitRevertPoint`), not a Homework-3 command stack.

## Evidence

- `Verified`: `ITimeline` exposes `snapshot()` and the three revert-point verbs,
  and the product documents the revert mechanism in its own 1,533-line design
  doc.
- `Verified`: Homework-2's ADR-0004 already established that clips and tracks
  use stable IDs rather than QML indexes, and that engine events must not
  silently rewrite edit data.
- `Inferred`: re-reading a whole snapshot per command is fast enough at
  homework scale. If it is not, the fix is incremental snapshot diffing, not
  letting QML hold engine state.

## Consequences

- There is one source of truth -- the engine, read through the realizer -- so
  the UI cannot drift from it by construction.
- A failed command leaves the previous snapshot on screen plus an error, which
  is why `HRESULT` checking is a rule and not a nicety.
- Homework-3 writes no undo stack, no inverse commands, and no transaction log.
  Undo correctness is the product's problem, already solved.
- Homework-2's ADR-0004 asserted a *project-owned* model as authoritative for a
  standalone editor. Homework-3 is product-hosted, so the engine plus
  `CTLObject` is authoritative instead. The two are not in conflict; they
  describe different hosting decisions.

## Verification

`VR-RENDER-01`, `VR-EDIT-02`, `VR-UNDO-01`.
