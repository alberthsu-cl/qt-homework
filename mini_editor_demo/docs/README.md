# Mini Editor engineering plan

This directory is the issue-driven engineering record for the mini editor. It
turns the prototype into a sequence of small, verifiable engine integrations.

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
| [MILESTONES.md](MILESTONES.md) | Scope, exit criteria, and sequencing |
| [ISSUE_GRAPH.md](ISSUE_GRAPH.md) | Ticket catalog and dependency graph |
| [VERIFICATION.md](VERIFICATION.md) | Reusable verification rules and evidence format |
| [adr/README.md](adr/README.md) | Architecture decision status and review rules |

## Immediate execution order

1. `ME-M0-01`: runtime activation probe.
2. `ME-M0-02`: MediaObj metadata probe.
3. `ME-M0-03`: preview transport spike.
4. Review ADR-0003 using the spike evidence.
5. `ME-M0-04`: SIMBA activation and teardown probe.
6. `ME-M0-05`: lock the M1 adapter contracts.

Only M0 and M1 are estimated as implementation-ready. M2 through M5 remain
dependency-shaped backlog until M0 resolves engine activation and rendering.

