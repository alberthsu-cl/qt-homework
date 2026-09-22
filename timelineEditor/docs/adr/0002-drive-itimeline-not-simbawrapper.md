# ADR-0002: Drive `ITimeline`, never `CSimbaWrapper`

- Status: Accepted
- Date: 2026-09-22

## Decision

All Homework-3 timeline editing goes through the existing `ITimeline`
abstraction, realized by `HeadlessTimeline`. Homework-3 code calls
`CSimbaWrapper` in exactly one place: the host's engine bring-up and teardown.

Movie create/snapshot/switch go through `IMovieHost`; every write is scoped by
`MovieScope` and names its bound movie.

## Evidence

- `Verified`: `ITimeline` (407 lines) already declares add/remove/trim/snapshot
  plus revert points, and `HeadlessTimeline` realizes it against SIMBA with no
  UI dependency at all.
- `Verified`: `CSimbaWrapper::Inst()` has 6,897 call sites; the wrapper is
  16,548 lines. There is no small subset to adopt.
- `Verified`: the Agentic Editing feature states the split as an invariant --
  handlers are pure transforms, the realizer is the only layer that touches
  SIMBA -- and documents it across 8,201 lines.
- `Inferred`: the four commands Homework-3 needs map onto existing `ITimeline`
  methods without extending the interface. To be confirmed in H3-05; if an
  extension is required it is appended, never inserted (see ADR-0003's ABI
  note).

## Consequences

- Homework-3 inherits the realizer's invariants and must honour them: serialized
  dispatch, `HRESULT` checked on every engine call, no SIMBA types above the
  realizer.
- A rename or signature change in `ITimeline` reaches Homework-3. Accepted: the
  alternative is a parallel vocabulary that drifts from the product's.
- Unit tests use `ISimbaWrapper` + `CSimbaSeamGuard` for SIMBA-free logic, and
  the product-hosted path only where a live engine is genuinely required.
- `VR-COUPLING-01` makes the rule mechanically checkable rather than aspirational.

## Verification

`VR-BIND-01`, `VR-EDIT-01`, `VR-EDIT-02`, `VR-COUPLING-01`.
