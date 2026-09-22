# MO-only runtime and probes

## Staging contract

`stage_mediaobj_runtime.ps1` copies the curated MediaObj-family files from
`external_bin/runtime/mediacache` into `bin_x64/runtime/mediacache`. It does
not stage SIMBA. The QtKit post-build script stages QtKit and Qt runtime files
beside `MediaPlayer.exe`.

The MediaObj binaries remain ignored by Git. A clone needs an approved source
of the PDR-owned runtime files before it can build the packaged executable.

## Registration-free activation

`MediaPlayer.manifest` maps MediaObj and its required MO COM classes to the
private runtime folder. Machine-wide registration is not required.

```powershell
.\bin_x64\MediaPlayer.exe --probe-playback-runtime
.\bin_x64\MediaPlayer.exe --probe-mediaobj "C:\path\to\sample.mp3"
```

The first command checks staged files, x64 architecture, COM activation, and
the `IMEDIAOBJ8` interface. The second command calls `LoadClip`, then records
available media type, dimensions, and duration in `%TEMP%\MediaPlayer.log`.

## Observed evidence

- MediaObj activates from the standalone `bin_x64` folder without a registry
  server entry.
- `Mahoroba.mp3` opens and returns audio metadata and duration.
- The supplied JPEG and MP4 samples currently fail `LoadClip`; the required
  image/video decode or filter dependencies are not yet known.

Do not add arbitrary PDR binaries to solve these gaps. Record each candidate
dependency, why it is needed, its architecture, and the resulting probe
evidence before adding it to the staging script.
