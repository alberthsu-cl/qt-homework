# Verification rules

## Rule catalog

| Rule | Requirement | Evidence |
|---|---|---|
| VR-BUILD-01 | `Debug|x64` builds with zero errors | MSBuild command and summary |
| VR-QML-01 | QML entry loads and all expected bindings are present | startup log |
| VR-RUNTIME-01 | MediaObj activation identifies exact success or failure stage | probe log |
| VR-RUNTIME-02 | SIMBA activation identifies every required queried interface | probe log |
| VR-MEDIA-01 | Video, audio, image, invalid, and missing inputs return expected metadata/status | test table |
| VR-PREVIEW-01 | Preview survives play, pause, seek, resize, minimize/restore, and DPI change | scripted/manual record |
| VR-TRANSPORT-01 | State follows engine results, not optimistic QML animation | state trace |
| VR-LIFE-01 | Twenty create/load/close cycles complete without a hang or surviving process | loop result and timeout |
| VR-THREAD-01 | Engine calls and callbacks obey ADR-0002 | review checklist and thread IDs in debug log |
| VR-ERROR-01 | User-visible error includes operation and recovery action; diagnostic includes `HRESULT` | screenshot/log pair |
| VR-PACKAGE-01 | Clean clone builds and stages only declared runtime files | staged manifest diff |

## Required M0 matrix

| Scenario | Expected result |
|---|---|
| Runtime present | MediaObj and SIMBA activation stages are reported |
| Runtime DLL missing | startup remains usable and identifies the missing file |
| Unsupported or unlicensed component | capability is disabled with a diagnostic result |
| Valid video | metadata contains duration and video stream capability |
| Valid audio | metadata contains duration and audio-only capability |
| Valid image | media type is image; transport capability is not invented |
| Missing media | no engine object remains loaded |
| Close during activation | process exits within five seconds |
| Repeated activation | twenty cycles complete within the test timeout |

## Evidence record

Each completed ticket adds a short evidence block to its pull request or commit
description:

```text
Ticket: ME-M0-00
Configuration: Debug|x64
Rules: VR-BUILD-01, VR-LIFE-01
Commands: <exact commands>
Inputs: <non-sensitive sample identifiers>
Result: pass/fail
Artifacts: <log, screenshot, or report paths>
Known gaps: <explicit gaps or none>
```

## Review gates

- An ADR-changing ticket cannot merge until the ADR status and consequences are
  updated.
- A proprietary API call requires a nearby adapter boundary and failure path.
- A new callback requires a cancellation and shutdown test.
- C++ or project-file changes require `Debug|x64` build verification.
- QML changes require startup-log verification and a visual layout check.
- Runtime-manifest changes require a clean staging verification.

