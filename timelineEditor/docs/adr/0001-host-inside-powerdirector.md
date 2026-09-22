# ADR-0001: Host Homework-3 inside PowerDirector

- Status: Accepted
- Date: 2026-09-22

## Decision

Homework-3 runs as a command-line mode of `PDR.exe`, alongside `clpdrMCP` and
`clpdrEngineTest`. It is not a standalone executable and does not stage its own
SIMBA runtime.

This breaks with Homework-1 and Homework-2, which are standalone and
Qt-SDK-free. The break is deliberate and is the price of a live engine.

## Evidence

- `Verified`: Homework-2 measured SIMBA construction faulting with
  `0xC0000005` from two separate locations, with a `SIMBA.dll` byte-identical
  to the product's (MD5 prefix `5a54b7a28005`). The binary was never missing.
- `Verified`: `CSimbaWrapper::InitializeSimba` depends on the CyberLink
  registry path, CLRC feature config, a font-manager secret key, AC3 codec
  init, and an `ISimbaDelegate` audio callback, then hard-fails unless four
  interfaces all resolve.
- `Verified`: `SetMessageDrainWindow(hWnd)` requires a real window and a
  running `GetMessage` pump; worker-thread SIMBA calls marshal across it.
- `Verified`: `clpdrEngineTest` already boots `PDR_debug.exe` headless, creates
  a real engine, and runs to completion without a launcher or a UI --
  the hosting pattern exists and ships.
- `Unknown`: whether an interactive session needs different engine settings
  than the headless test path uses (see ADR-0002 and H3-02).

## Consequences

- Homework-3 requires a built parent PDR checkout. It cannot be opened or run
  on its own, unlike the first two homeworks.
- Build and iteration cost rises to the product's, not a demo's.
- In exchange, the timeline is real: real SIMBA state, real `CTLObject`, real
  read-back, and no fake engine to keep honest.
- The top-level repository README's "standalone, no PDR checkout needed" claim
  stops being true for all projects and must be scoped to the first two.

## Alternatives rejected

| Alternative | Why not |
|---|---|
| Standalone exe with a fake in-memory engine | Teaches the boundary but never proves it against the engine that makes it hard. |
| Standalone exe attempting full SIMBA bring-up | The measured `0xC0000005` path; requires reproducing product state and licensing outside the product. |
| Extend `src/ui/WorkSpace/` | 68,527 lines of MFC timeline UI; the thing being replaced, not extended. |

## Verification

`VR-HOST-01`, `VR-HOST-02`.
