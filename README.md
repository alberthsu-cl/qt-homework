# qt-homework

Onboarding notes for PowerDirector (PDR), written while getting a fresh clone
building and then working out how the Qt integration actually fits together.

Audience: an RD who is new to this codebase.

> **Scratch/personal notes — not part of the product tree.** Nothing here is
> referenced by the build, and none of it is indexed by `docs/`. If a document
> here proves generally useful, the right home is `docs/` under the relevant
> feature folder (see `docs/features/README.md` and the `docs-index-sync`
> workflow), not this folder.

---

## Contents

| File | What it covers |
|---|---|
| [`BUILD_SETUP.md`](BUILD_SETUP.md) | Why a fresh clone fails to link with `LNK1104: cannot open file 'CtrlFactoryD.lib'`, and the eModule step that fixes it. Includes the failure modes hit on a real first run. |
| [`QTKIT_ARCHITECTURE.md`](QTKIT_ARCHITECTURE.md) | How the MFC application hosts Qt 6 QML through `QtKit.dll` — module map, boot sequence, binding model, window embedding, threading rules, dual-mode switches, resource pipeline. Mermaid diagrams render in VS Code (the `bierner.markdown-mermaid` extension `bootstrap.bat` installs). |
| [`QtKit-Bridge.html`](QtKit-Bridge.html) | The same architecture brief as a self-contained HTML page with hand-drawn SVG diagrams. Open it directly in a browser. |
| [`mfc_qml_demo/`](mfc_qml_demo/) | A working ~1400-line MFC app that loads `QtKit.dll`, hosts a QML canvas in its frame, and talks to it both ways. Split into controller / view-controller / QML the way a real PDR feature is, so the layering is visible and not just described. Builds with VS2022 against this repo alone - no Qt SDK needed. Its README lists eight failure modes that each cost a build cycle. |

Published copy of the HTML brief (same content, shareable link):
https://claude.ai/code/artifact/7d568ad8-b78b-4a70-b468-ddd410f74bb0

---

## The 60-second version

**Qt is not linked into PowerDirector.** No Qt header, no Qt symbol, no `moc`
pass anywhere in this repo. The entire Qt world is sealed inside `QtKit.dll`,
loaded at runtime with `LoadLibraryEx` and reached through exactly two
`extern "C"` exports plus pure-virtual structs in
`src/external include/QtKit/Interface.h`.

Four consequences you will meet in week one:

1. **It runs on its own thread.** `qtKit->run(async=true)` returns *before* the
   QML context exists. Use `RunOnQtThreadAsync` / `RunOnMainThreadAsync`; never
   touch the other side directly.
2. **QML objects are addressed by string.** `context->button("dpPlayBtn")`.
   Rename an `objectName` and nothing fails to compile — it throws
   `QmlObjectNoBoundError` at runtime.
3. **Both UIs ship in every build.** A per-module registry DWORD under
   `HKCU\Software\CyberLink\PowerDirector25\QtMigration\` picks which is wired
   up. Timeline is still **OFF** — the MFC ruler is live and kept in sync.
4. **`src/ui/common/MigrateCandy.h` is the front door.** `Say()` / `Listen()` is
   the vocabulary every migrated module speaks.

---

## Two corrections to existing docs

Both verified against source at `e82e4c817e`, both still unfixed upstream:

- **`AGENTS.md` names the wrong registry root.** It documents
  `...\CyberLink\PowerDirector24\QtMigration\Library`. The live root is
  **`PowerDirector25`** (`STR_PDR_FULL_NAME`, `src/ui/definition/PDRPathDef.cpp:16`).
  `PowerDirector24` is now the first entry in `STR_PDR_FULL_NAME_PREV`, used for
  settings migration from older installs.

- **A `Debug|x64` build loads the *release* `QtKit.dll`.** `QtKitWrapper`
  branches on `#ifdef DEBUG`, but no project here defines bare `DEBUG` — MSVC
  uses `_DEBUG`, which is what all the `.vcxproj` files set. The branch is dead,
  so `QtKitd.dll` ships at 5.2 MB and is never opened through this path.

---

## Running the demo

```bat
cd qt-homework\mfc_qml_demo
powershell -ExecutionPolicy Bypass -File .\stage_runtime.ps1
msbuild MfcQmlDemo.vcxproj /p:Configuration=Debug /p:Platform=x64
bin_x64\MfcQmlDemo.exe
```

It runs from its own self-contained `bin_x64\` - 116 files, 56 MB, staged out
of the product output and loading nothing from it. See
[`mfc_qml_demo/README.md`](mfc_qml_demo/README.md) for the dependency inventory
and the eight gotchas.

## Next areas to write up

Not started yet — listed so the gap is visible:

- **WorkSpace / Timeline** — the largest unmigrated module, and the one whose
  dual-maintenance burden the Qt migration is currently paying for.
- **Agentic Editing API** — `agentic_editing_harness/` submodule plus
  `docs/features/agentic-editing/`; has its own `add-agentic-tool` skill.
- **Engine boundary** — CES / Simba / the `IMOSourceProtocol` and
  `ISimbaSourceProtocol` delegates that feed preview textures into QML.
