# Timeline editor engineering plan

This directory is the issue-driven engineering record for Homework-3: a
timeline editing surface hosted **inside PowerDirector**.

Homework-2 dropped timeline editing and became a MediaObj source player. This
project picks that scope back up, on a hosting decision that makes a live
engine possible.

## Working model

Every implementation change must trace through this graph:

```text
Requirement -> ADR -> milestone -> ticket -> verification rule -> evidence
```

The documents use three evidence labels:

- `Verified`: demonstrated by repository code, a passing build, or a recorded
  runtime check.
- `Inferred`: supported by nearby PDR usage but not yet exercised by this
  homework.
- `Unknown`: requires a proof, an owner answer, or a licensing check.

No ticket is complete only because code was written. Its listed verification
rules and evidence must also be complete.

## Index

| Document | Purpose |
|---|---|
| [SIMBA_INTEGRATION.md](SIMBA_INTEGRATION.md) | **Read this first.** How SIMBA is integrated into PowerDirector: the seven layers, the measured coupling, the bring-up contract, and why a standalone executable cannot host the engine. |
| [HOMEWORK_3_PLAN.md](HOMEWORK_3_PLAN.md) | Scope, delivery sequence, tickets, and verification rules |
| [MILESTONES.md](MILESTONES.md) | Goal, boundaries, M1-M4, and exit criteria |
| [adr/README.md](adr/README.md) | Decision status, review rules, and the open questions |

## Immediate execution order

1. `H3-01`: command-line mode, host window, message pump.
2. `H3-02`: SIMBA bring-up and ordered teardown.
3. `H3-03`: movie binding and `HeadlessTimeline` wiring.
4. `H3-04`: QML timeline rendering a snapshot.
5. `H3-05`: add track, add clip, remove, ripple trim.
6. `H3-06`: composed preview playback.
7. `H3-07`: revert-point undo, failure paths, shutdown.

## The one rule worth repeating

`CSimbaWrapper::Inst()` already has 6,897 call sites in the product. Homework-3
adds none outside its own engine bring-up -- everything else goes through
`ITimeline`. `VR-COUPLING-01` checks it mechanically.
