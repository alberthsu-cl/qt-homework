# Media Player engineering plan

This directory is the issue-driven engineering record for the media player.
Homework-2 is a small MediaObj source player, not a timeline editor.

## Working model

Every implementation change must trace through this graph:

```text
Requirement -> ADR -> milestone -> ticket -> verification rule -> evidence
```

The documents use three evidence labels:

- `Verified`: demonstrated by repository code, a passing build, or a recorded
  runtime check.
- `Inferred`: supported by nearby PDR usage but not yet exercised in this
  standalone executable.
- `Unknown`: requires a proof, owner answer, or licensing/deployment check.

No ticket is complete only because code was written. Its listed verification
rules and evidence must also be complete.

## Index

| Document | Purpose |
|---|---|
| [HOMEWORK_2_PLAN.md](HOMEWORK_2_PLAN.md) | Scope and delivery sequence |
| [MILESTONES.md](MILESTONES.md) | Homework-2 goal, boundaries, and exit criteria |
| [ISSUE_GRAPH.md](ISSUE_GRAPH.md) | Active H2 ticket catalog and dependency graph |
| [VERIFICATION.md](VERIFICATION.md) | MediaObj source-player verification rules |
| [adr/README.md](adr/README.md) | Architecture decision status and review rules |

## Immediate execution order

1. `H2-01`: media catalog and selected asset.
2. `H2-02`: MediaObj source adapter.
3. `H2-03`: MediaObj preview surface.
4. `H2-04`: source playback controls.
5. `H2-05`: read-only asset information.
6. `H2-06`: failure and shutdown checks.

SIMBA, timelines, editing, effects, and persistence are outside Homework-2.
