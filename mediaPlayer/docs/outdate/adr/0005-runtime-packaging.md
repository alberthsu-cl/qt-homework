# ADR-0005: Runtime packaging

- Status: Accepted
- Date: 2026-09-21

## Decision

The executable remains Qt-SDK-free. Build output stages the committed QtKit
runtime plus a curated proprietary playback-runtime manifest copied from the
parent PDR checkout. Build files do not search arbitrary machine paths.

MediaObj and SIMBA activation uses registration-free COM declarations embedded
from `MediaPlayer.manifest`, following PDR's private-manifest deployment
model. The demo does not register these classes machine-wide.

## Consequences

- Runtime files live beside the built executable in deterministic subfolders.
- The copy script is idempotent and supports manifest verification.
- Missing proprietary runtime disables playback with an actionable diagnostic;
  it does not prevent the UI shell from starting.
- Adding a binary requires its purpose, source location, architecture, and
  verification evidence to be recorded.
- Licensing and redistribution approval are external release gates; repository
  presence alone is not approval.

## Evidence

- Verified: the project already stages QtKit and curated SIMBA/MediaObj, DSP,
  and CES files through PowerShell scripts.
- Unknown: the minimal playback subset and redistribution terms require M0 and
  product-owner confirmation.

## Verification

`VR-PACKAGE-01`, `VR-RUNTIME-01`, `VR-RUNTIME-02`.
