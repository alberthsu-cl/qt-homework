# How SIMBA is integrated into PowerDirector

Survey taken 2026-09-22 against the `hw/alberth` working tree of the parent PDR
checkout. Every number below was measured, not estimated; the commands are in
[Appendix A](#appendix-a-how-these-numbers-were-taken) so they can be re-run
when the product moves.

This document exists because Homework-3 edits a timeline, and a timeline in
this product means SIMBA. It is the map that decides what Homework-3 may build
on and what it must not touch.

## 1. The short version

SIMBA is an in-process COM video engine. The product does not talk to it
directly; it talks to a 12,000-line singleton wrapper, and 6,897 call sites in
the product talk to that singleton. Two recent layers -- a narrow test seam and
a timeline abstraction with a UI-free realizer -- are the only places where that
coupling has been brought under control. Homework-3 builds on those two layers
and on nothing else.

SIMBA cannot be constructed in a standalone executable. That is a measured
result, not a caution; see [section 5](#5-why-a-standalone-executable-cannot-host-simba).

## 2. The seven layers

Bottom to top. "Lines" is C++ source in the parent checkout.

| # | Layer | Where | Lines |
|---:|---|---|---:|
| 1 | Engine binary, in-process COM server | `runtime/simba/SIMBA.dll` | 2,502,656 bytes |
| 2 | COM interface headers | `src/Simba/` | 5,682 |
| 3 | `CSimbaWrapper` singleton | `src/ui/mainpanel/SimbaWrapper*` | 16,548 |
| 4 | `ISimbaWrapper` test seam | `src/ui/mainpanel/ISimbaWrapper.h` | 57 |
| 5 | Clip / effect data model (`CTLObject`) | `src/ui/common/EditBaseObj/` | 41,978 |
| 6 | `ITimeline` abstraction + realizers | `src/ui/common/AgenticEditing/Timeline/` | 10,135 |
| 7 | Legacy MFC timeline UI | `src/ui/WorkSpace/` | 68,527 |

### Layer 1 -- the engine binary

`SIMBA.dll` is activated registration-free through an embedded manifest; the
product never registers the class machine-wide. The copy staged into
Homework-2's `bin_x64/runtime/simba/` is byte-identical to the product's
(same size, same MD5 prefix `5a54b7a28005`). **A missing DLL was never the
problem** -- which is what makes section 5 worth reading.

### Layer 2 -- the COM surface

`src/Simba/` holds the engine's declared interfaces, no implementation:

| Header | Lines | Holds |
|---|---:|---|
| `SIMBAData.h` | 2,274 | structs, enums, `TIMELINE_UNIT` |
| `ISIMBA.h` | 1,481 | the interface declarations |
| `TKProfile.h` | 627 | produce/transcode profiles |
| `AttributeGUID.h` | 454 | effect + clip attribute GUIDs |
| `SIMBAGUID.h` | 175 | `CLSID_SIMBA`, `CLSID_TIMELINE_UNIT`, IIDs |

Creation is `CoCreateInstance(CLSID_SIMBA, ..., IID_ISimbaMovieEdit3)`, after
which the wrapper `QueryInterface`s roughly ten further interfaces off the same
object -- `ISimbaProduct`, `ISimbaCache`/`ISimbaCache2`, `ISimba360video`,
`ISimbaUltraSpeedSeeking`, `ISimbaAudioScrubbing`, `ISimbaMovieEdit10`/`11`,
`ISimbaDelegate`, `ISIMBA_FontManager_Attribute`, `ISimbaInfoS`.

A clip is moved across the boundary as an `ITimelineUnit` COM object
(`CLSID_TIMELINE_UNIT`), created per operation.

### Layer 3 -- `CSimbaWrapper`, the coupling problem

```text
SimbaWrapper.cpp        12,093      SimbaWrapperOOApi.cpp    2,526
SimbaWrapper.h           1,225      SimbaWrapperOOApi.h        170
SimbaAutoObj.cpp/.h        316      SimbaWrapperSeam.cpp/.h    161
                                    ISimbaWrapper.h             57
                                                     total  16,548
```

`CSimbaWrapper::Inst()` is called from **6,897 sites** across `src/`. It is a
singleton reached directly from nearly every UI module, and it is the single
largest reason the original Homework-2 timeline plan was abandoned: there is no
small piece of it to take.

Directories leaning on it hardest: `AgenticEditing/Handler` (139 files),
`mainpanel` (54), `EditBaseObj` (44), `WorkSpace` (24), `displaypanel` (21).

### Layer 4 -- `ISimbaWrapper`, the seam that works

57 lines, 15 pure virtuals, and the pattern Homework-3 should imitate:

```cpp
COMMON_API ISimbaWrapper* GetISimba();                 // defaults to the real singleton
COMMON_API void           SetISimba(ISimbaWrapper*);   // tests install a mock
class CSimbaSeamGuard { ... };                         // RAII, restores on every exit path
```

Production code calls `GetISimba()->X()` instead of `CSimbaWrapper::Inst().X()`.
`CSimbaWrapperAdapter` in `SimbaWrapperSeam.cpp` is the only translation unit
that knows about both types. **`CSimbaWrapper` itself is not modified by any of
this**, which is exactly why the seam was affordable to add.

Two rules the header states and enforces on itself:

- **Append-only after the destructor.** The implementation lives in
  `common.dll` behind an exported `GetISimba()`, while callers link the import
  lib. Inserting a member above that line renumbers every vtable slot after it,
  and a caller built against the new header calls the wrong function on a
  not-yet-rebuilt DLL.
- **Grow one method at a time**, only when a system under test actually calls it.

### Layer 5 -- the data model

`CTLObject : CEditBaseObject` (`src/ui/common/EditBaseObj/EditBaseObj.h:122`) is
the product's stored clip: transforms, effects, keyframes, volume. The
`EditBaseObj` directory is 41,978 lines. `CJyaian` (`src/projectio/`, 2,811
lines) maps virtual movie tracks and owns project IO.

A clip therefore exists in **two** places -- `CTLObject` on the product side and
`TIMELINE_UNIT` inside the engine -- and the engine test harness asserts both
copies on purpose. Treat "the model" and "what SIMBA believes" as two things
that can disagree.

### Layer 6 -- `ITimeline`, the layer Homework-3 targets

`src/ui/common/AgenticEditing/Timeline/` is a timeline-editing abstraction that
already exists, already drives SIMBA, and is already documented in 8,201 lines
under `docs/features/agentic-editing/`.

`Interface/ITimeline.h` (407 lines) declares the editing vocabulary:

```text
addTrack / addTrackPair        addClip / addClipWithAudio / addSticker
insertTitle / insertSubtitles  addEffectClip / setEffectClipRange
removeClips                    trimClipRippleAll
getClips / snapshot            clearTimeline / resetTimeline
markDirty / consumeDirty       armRevertPoint / revertToPoint / commitRevertPoint
adoptMovieContent              insertMovieContentAt        timelineEnd
```

Two shipping realizers:

| Realizer | Character |
|---|---|
| `HeadlessTimeline` | **no UI at all**; track/clip SIMBA ops against a bound movie |
| `MainPanelTimeline` | the live product timeline, with workspace bridges |

plus `SwitchableTimeline`, `TimelineEngineOps`, `TimelinePolicy` and
`TimelineWriteScope`.

Its invariants matter to Homework-3 because they are load-bearing:

- **Handler = pure transform; realizer = the one that touches SIMBA.** SIMBA
  calls must not leak upward.
- **Movie create/snapshot/switch only behind `IMovieHost`.** Realizers never
  call SIMBA movie APIs directly; every write is scoped by a `MovieScope` RAII
  and names its bound movie rather than switching the current one.
- **Tool dispatch is serialized on one queue.** Concurrent handlers mean
  re-entrant SIMBA ops and races in the realizer.
- **Check the `HRESULT` of every engine call.** An unconditional success return
  turns an empty produce into a green result.
- `ITimeline`'s pure-vs-defaulted split is contractual: anything both shipping
  realizers must answer is pure, so a new realizer cannot silently inherit a
  no-op for a question the product already asks.

### Layer 7 -- the legacy MFC timeline UI

`src/ui/WorkSpace/` is 68,527 lines of MFC timeline rendering and interaction --
`TimeLineWnd`, `TrackWnd`, `BaseTrack`/`MainTrack`/`AudioTrack`/`SubtitleTrack`,
keyframe editors, Z-order handlers. Homework-3 does not extend, subclass or
reuse this. It is listed so its size is on the record as the alternative that
was not chosen.

## 3. Bring-up: what it takes to get a live engine

`CSimbaWrapper::InitializeSimba()` (`SimbaWrapper.cpp:325`) is where the real
dependency set shows up. In order:

1. `CoInitializeEx`, then `CoCreateInstance(CLSID_SIMBA, IID_ISimbaMovieEdit3)`
2. `SetCLRegPath()` -- the CyberLink registry path the engine reads
3. `QueryInterface` for the ten-odd secondary interfaces
4. `CLRCWrapper` runtime config -- feature flags (`DisableCLVR`,
   `EnableAudGapFilling`), overridable from `HKCU` registry
5. `ISimbaDelegate` audio volume callbacks (`SimbaRTVol`, per-channel volume)
6. `CFontMgrWrapper` -- font pack folders **and a secret key**
7. `GetAC3Helper()->InitAC3DLL()` -- audio codec
8. Registry `VideoRenderer` (`EVR` default, `VMR9`, `VMR7`)
9. Optional `IBoomerangClient` telemetry + logger prefix

It then hard-fails unless **all four** of `m_pISimbaMovie`, `m_pISimbaProduct`,
`m_pISimbaCache` and `m_pISimbaMovieEdit11` are non-null, logging which one was
missing so a field dump can separate a `CoCreateInstance` failure from a later
`QueryInterface` failure. `ISimbaMovieEdit11` also version-gates the DLL: an
older `SIMBA.dll` is refused outright.

### The minimal sequence that is known to work

From `EngineTestHostWindow::InitializeSimbaEngine()` -- the shortest bring-up in
the product, and the one Homework-3 copies:

```cpp
CSimbaWrapper::Inst();
CSimbaWrapper::Inst().EnsureSimbaCoCreated(NULL, szLoggerPath);  // idempotent, thread-safe
CSimbaWrapper::Inst().SetPreviewFrameRate(...);
CSimbaWrapper::Inst().SetMessageDrainWindow(m_hWnd);   // <-- needs a real HWND
CSimbaWrapper::Inst().SetSimbaReadyEvent(TRUE);
CSimbaWrapper::Inst().SetSuppressEditMode(true);
CSimbaWrapper::Inst().InitialPreviewCanvas();
CSimbaWrapper::Inst().EnsureMediaObjWrapperCreated();  // these own CWnds and must be
ShadowMgr::Inst();                                     // first-created on the main thread
CAutoCutoutMgr::Inst();
```

### Threading

- The **main thread** owns COM and SIMBA operations.
- `SetMessageDrainWindow(hWnd)` gives SIMBA a window to marshal through; worker
  threads calling SIMBA synchronously marshal across it, so **the host must run
  a `GetMessage` pump for the whole session**. No pump, no engine.
- Objects that own `CWnd`s must be first-created on the main thread.
- PDR additionally runs a Qt thread that can drive SIMBA/MediaObj playback; the
  parent repo's `docs/common/thread-safety.md` owns those rules.

## 4. How the product hosts a non-UI SIMBA session

PDR already boots into headless one-shot modes, and this is the extension point
Homework-3 uses. `CVideoEditorTLApp::InitInstance` (`src/VideoEditorTL.cpp`)
parses a mode off the command line and branches before any
launcher/mainpanel/splash work:

| Mode | Argv | Host | Purpose |
|---|---|---|---|
| MCP | `clpdrMCP` | `CMcpHostWindow` | permanent pipe server |
| Engine test | `clpdrEngineTest` | `EngineTestHostWindow` | one-shot live-engine tests, `_DEBUG` only |
| Batch | (own switch) | `CBatchHostWindow` | fine-tune batch runs, `_DEBUG` only |

All of them share one prelude, then pump:

```text
InitMUITransfer -> LoadResouceFile -> AfxSetResourceHandle -> InitSimkey
  -> InitTruncationLogger -> InitDpiSettings -> InitGrafix -> LoadCtrlDefaultFont
  -> InitUIMgr(TRUE) -> InitMiscLayoutMgr -> UpdateLocalFontResource
  -> MainThreadExecutor::Shared()      (resource-backed main-thread marshaller)
  -> new <host window>                 (registers a hidden HWND, brings up SIMBA)
  -> while (GetMessage(...)) { TranslateMessage; DispatchMessage; }
```

`clpdrEngineTest` proves the shape is real: tests compile into
`common_debug.dll`, and a run is `PDR_debug.exe` booted headless that creates a
real engine, dispatches real tools, asserts engine state, writes artifacts, and
exits with the failure count -- **without a launcher or a UI**.

The registration trap that comes with it: because this code lives in
`common.dll`, a new source file must be added to `common.vcxproj` *and*
`common.vcxproj.filters`, or it silently does not exist.

## 5. Why a standalone executable cannot host SIMBA

Homework-2 attempted it and recorded the result: SIMBA construction faults with
`0xC0000005`, from both the private build directory and beside `PDR.exe`, while
MediaObj activated cleanly from both. That is why Homework-2 became a
MediaObj-only source player.

Sections 3 and 4 explain it. The engine is not a self-contained COM object; it
is the centre of an initialization contract that assumes:

| Requirement | Supplied by | Present in a bare demo exe |
|---|---|---|
| CyberLink registry path | `SetCLRegPath` + installed product state | no |
| CLRC feature config | `CLRCWrapper` | no |
| Font pack folders + secret key | `CFontMgrWrapper` | no |
| AC3 codec init | `GetAC3Helper()` | no |
| MUI resources, grafix, UI mgr | the `InitInstance` prelude | no |
| Message drain window + pump | host window, main thread | no |
| `CWnd`-owning singletons | `ShadowMgr`, `CAutoCutoutMgr` | no |

Staging more DLLs does not close this gap; the missing pieces are product state
and a host, not files. Reproducing the contract outside the product means
reproducing the product -- the scope explosion Homework-3 is defined to avoid.

## 6. What Homework-3 takes from this

- **Host inside PowerDirector.** Add a command-line mode beside `clpdrMCP` /
  `clpdrEngineTest` and reuse the shared prelude verbatim. Do not try to make a
  standalone `.exe` construct SIMBA.
- **Drive `ITimeline`, never `CSimbaWrapper`.** The editing vocabulary already
  exists and is documented. Adding a 6,898th `Inst()` call site is the failure
  mode to avoid.
- **Follow the realizer split.** UI and controllers hold no SIMBA types; every
  engine call sits behind the realizer, scoped by `MovieScope`, `HRESULT`
  checked.
- **Use `ISimbaWrapper` + `CSimbaSeamGuard` for unit tests**, and keep
  SIMBA-free logic in plain gtest. Only assertions that need a live engine
  justify the product-hosted cost.
- **Leave `WorkSpace` alone.** 68,527 lines of MFC timeline UI is the thing
  being replaced, not extended.

## Appendix A: how these numbers were taken

```bash
wc -l src/Simba/*.h                                   # layer 2
wc -l src/ui/mainpanel/SimbaWrapper* src/ui/mainpanel/ISimbaWrapper.h \
      src/ui/mainpanel/SimbaAutoObj.*                 # layer 3
grep -rn "CSimbaWrapper::Inst()" src --include=*.cpp --include=*.h | wc -l
find src/ui/common/EditBaseObj -name "*.cpp" -o -name "*.h" | xargs wc -l | tail -1
wc -l src/ui/common/AgenticEditing/Timeline/*         # layer 6
find src/ui/WorkSpace -name "*.cpp" -o -name "*.h" | xargs wc -l | tail -1
md5sum bin_x64/PowerDirector/runtime/simba/SIMBA.dll
```

Primary sources, in the parent checkout:

- `src/ui/mainpanel/SimbaWrapper.cpp:325` -- `InitializeSimba`
- `src/ui/mainpanel/ISimbaWrapper.h`, `SimbaWrapperSeam.{h,cpp}` -- the seam
- `src/ui/common/AgenticEditing/Interface/ITimeline.h` -- the abstraction
- `src/ui/common/AgenticEditing/Test/Engine/EngineTestHostWindow.cpp:82` -- minimal bring-up
- `src/VideoEditorTL.cpp` -- headless mode parsing and prelude
- `docs/features/agentic-editing/README.md` -- the feature's own index
- `docs/features/agentic-editing/Engine_Test_Mechanism.md` -- product-hosted test topology
