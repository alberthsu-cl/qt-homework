# ADR-0006: MediaObj-only Homework-2 source player

- Status: Accepted
- Date: 2026-09-21

## Context

The standalone package proves registration-free MediaObj activation, but the
curated SIMBA package faults during construction. PDR build scripts identify
additional product-owned runtime artifacts that are absent from this checkout.
The editor workflow must remain useful while that integration is unresolved.

## Decision

Homework-2 will implement a media library, selected-asset state, MediaObj
source playback, preview transport, and read-only asset information. It has no
timeline, editing commands, effects, composition, or project persistence.

SIMBA integration moves to a deferred adapter track. The diagnostic stays
available through an explicit command-line probe and is never required for
normal editor startup.

## Consequences

- The QML workflow removes timeline and effect controls.
- Playback is real source playback only after MediaObj verification succeeds.
- Asset information is the Properties panel's only Homework-2 content.
- A future PDR adapter must satisfy its own activation, lifecycle, preview,
  and media-verification rules before adding SIMBA composition.

## Alternatives considered

- Continue reverse-engineering SIMBA startup in the standalone app: rejected
  for Homework-2 because the required product runtime package is incomplete
  and crashes during construction.
- Continue with a simulated timeline editor: rejected because it adds scope
  without meeting the Homework-2 source-playback learning objective.

## Verification

`VR-MEDIA-01`, `VR-PREVIEW-01`, `VR-TRANSPORT-01`, `VR-LIFE-01`, and
`VR-RUNTIME-01` for the opt-in diagnostic.
