# Architecture decision records

## Status vocabulary

- `Proposed`: implementation must not depend on the decision yet.
- `Accepted`: new work follows the decision.
- `Rejected`: retained to prevent the same option being reconsidered without
  new evidence.
- `Superseded`: replaced by a later ADR.

## Review checklist

- Is the decision supported by `Verified`, `Inferred`, and `Unknown` evidence?
- Does it state ownership, thread, lifetime, and failure behavior?
- Are alternatives and reversal cost recorded?
- Is at least one verification rule attached?
- Does new spike evidence require a status change?

## Index

| ADR | Status | Decision |
|---|---|---|
| [ADR-0001](0001-playback-engine-responsibilities.md) | Accepted | MediaObj owns source preview; SIMBA owns timeline playback |
| [ADR-0002](0002-threading-and-lifetime.md) | Accepted | Main thread owns engine lifetime and commands |
| [ADR-0003](0003-preview-transport.md) | Proposed | Select native window or frame callback after M0 spike |
| [ADR-0004](0004-timeline-source-of-truth.md) | Accepted | Native C++ model is authoritative |
| [ADR-0005](0005-runtime-packaging.md) | Accepted | Curated staged runtime, no Qt SDK |

