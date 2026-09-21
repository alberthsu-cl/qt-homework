# ADR-0004: Timeline source of truth

- Status: Accepted
- Date: 2026-09-21

## Decision

A project-owned native C++ timeline model is authoritative. QML renders
snapshots and emits commands. SIMBA graphs are derived runtime state and may be
rebuilt; they are not the persisted edit model.

## Consequences

- Clips and tracks use stable IDs rather than QML indexes.
- Split, trim, reorder, ripple delete, and property changes are commands against
  the native model.
- Undo/redo stores model transactions.
- Engine events update playback state but cannot silently rewrite edit data.
- Persistence serializes the model, not engine interface state.

## Evidence

- Verified: the current prototype already keeps media, timeline presence,
  playback, and split state in `CMiniEditorController`.
- Inferred: a richer dedicated model is required before M2 so controller state
  does not become an untestable collection of UI flags.

## Verification

Model unit tests in M2 and command/undo tests in M3.

