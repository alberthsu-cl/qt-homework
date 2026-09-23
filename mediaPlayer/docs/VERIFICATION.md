# Homework-2 verification rules

| Rule | Requirement | Evidence |
|---|---|---|
| VR-BUILD-01 | `Debug|x64` builds with zero errors | MSBuild summary |
| VR-QML-01 | The simplified QML shell loads with Media Library, Preview, and Properties only | startup log and screenshot |
| VR-RUNTIME-01 | The opt-in probe identifies staged-file, COM, activation, and interface failures | probe log |
| VR-MEDIA-01 | Video, audio, image, invalid, and missing inputs return correct normalized metadata/status | test matrix |
| VR-PREVIEW-01 | Preview survives source replacement, resize, hide/show, and close | manual or scripted record |
| VR-TRANSPORT-01 | Play, pause, stop, seek, and position reflect MediaObj results | state trace |
| VR-LIFE-01 | Twenty load/play/close cycles exit without a hang or surviving process | loop result |
| VR-ERROR-01 | User-visible errors name the failed operation and recovery action; diagnostics include HRESULT | screenshot and log |

## Required matrix

| Scenario | Expected result |
|---|---|
| Valid video | Metadata and native source preview are available. |
| Valid audio | Metadata and transport work; preview explains audio-only behavior. |
| Valid image | Asset information is shown; no unsupported transport is invented. |
| Invalid file | No MediaObj object remains loaded; Properties shows an actionable status. |
| Missing file | Import/load fails cleanly without changing the prior valid selection. |
| Replace during playback | Prior source unloads before the new source is loaded. |
| Close during load or play | Process exits within five seconds. |
| Runtime missing | QML shell stays usable and reports that playback is unavailable. |

## Current baseline

| Probe | Result | Evidence |
|---|---|---|
| MediaObj source activation | Pass | Registration-free MediaObj activation and interface query pass without `runtime\\simba`. |
| `Mahoroba.mp3` source open | Pass | `LoadClip` returns `S_OK`; type is audio and duration is available. |
| `Food.jpg` image open | Pass by QML route | Image preview bypasses MediaObj playback. |
| `Mountainbiker.mp4` source open | Pass | `LoadClip` returns `S_OK` with the staged MP4 splitter and H.264 decoder. |
| `Skateboard 01.mp4` source open | Pass | `LoadClip` returns `S_OK` with the same minimal runtime. |
| Native preview graph | Pass | `--probe-mediaobj-preview` verified preview/EVR graph construction, `SetDisplayWnd`, MP4 loading, and metadata against a private HWND. |

Interactive playback and transport remain future acceptance criteria. The
preview probe verifies graph construction and native-window attachment only.

## Evidence record

```text
Ticket: H2-00
Configuration: Debug|x64
Rules: VR-BUILD-01, VR-MEDIA-01
Commands: <exact commands>
Inputs: <non-sensitive sample identifiers>
Result: pass/fail
Artifacts: <log or screenshot path>
Known gaps: <explicit gaps or none>
```
