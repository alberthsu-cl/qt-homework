# First build on a fresh clone — the eModule step

What blocks a brand-new clone from linking, why the error message doesn't say
so, and how to fix it. Written up from an actual first-build session on
2026-09-14.

---

## The symptom

A clean `git clone` compiles fine and then fails at link, on every project at
once:

```
1>LINK : fatal error LNK1104: cannot open file 'CtrlFactoryD.lib'
2>LINK : fatal error LNK1104: cannot open file 'UILayoutMgrD.lib'
9>LINK : fatal error LNK1104: cannot open file 'common_debug.lib'
========== Build: 0 succeeded, 14 failed ==========
```

Fourteen failures, **one root cause**. `common.vcxproj` dies on
`CtrlFactoryD.lib`, so it never produces `common_debug.lib`, so `Capture` fails
too. Everything else is that same cascade.

## The cause

`.gitignore` line 7 excludes `src/external include/x64/`. The eleven import
libraries in it are distributed through **eModule**, never through git:

| | |
|---|---|
| `CtrlFactory.lib` / `CtrlFactoryD.lib` | the one the error names |
| `UILayoutMgr.lib` / `UILayoutMgrD.lib` | ships inside the same CtrlFactory package |
| `libwebp` / `libwebpdemux` / `libwebpmux` + `D` variants | 6 files |
| `detours.lib` | |

`src/external include/dx9sdk/x64/` is excluded the same way and holds the 8
`strmbase*` files.

A clean machine has **no such directory at all**. The manifest that drives the
download is [`emma.yaml`](../emma.yaml) at the repo root — see lines 270–280 for
the entries that target `src\external include\x64`.

This is already written up in
[`docs/unit-testing/RUNNER-SETUP.md` §2.1](../docs/unit-testing/RUNNER-SETUP.md),
which quotes the exact error text.

## The fix

`emma.exe` is the eModule client. It is **not** in the repo (untracked, ~7.5 MB);
on a set-up machine it lives at `C:\Windows\System32\emma.exe` with its cache
under `%USERPROFILE%\.emma\`.

emma is **per-clone**: it reads `emma.yaml` from the *current directory* and
downloads into that tree. Having run it in another clone does nothing for a new
one.

```bash
cd /d/PDR/pdr-win-ui-videoeditor && emma
```

Then the post-download fixup, which flattens some versioned folder names:

```bash
cd /d/PDR/pdr-win-ui-videoeditor && python emma_move_files.py
```

Verify — you want **11** `.lib` files:

```bash
ls "/d/PDR/pdr-win-ui-videoeditor/src/external include/x64/"*.lib
```

Then build:

```bash
build_solution.bat Build Debug x64
```

## Gotchas hit during the real run

### Don't use `emodule_update.bat`

It hard-matches the string `Python 3.8`:

```bat
python --version 2>NUL | findstr /C:"Python 3.8" >NUL
if %errorlevel% neq 0 ( echo [Error] Python 3.8 is not installed... )
```

With Python 3.13 installed it aborts at the version check — even though
`emma_move_files.py` runs fine on 3.13. Run the two commands above by hand
instead. It also ends in `pause`, which is why
`RUNNER-SETUP.md` says never to call it from CI.

### emma saturates its own connection pool

The first run took 33 minutes and failed **161 of ~280** modules with a mix of:

```
dial tcp: lookup ecl.cyberlink.com: no such host
failed to query status: ... EOF
wsarecv: connected host has failed to respond
```

The `no such host` lines are misleading — the hostname resolves fine when
queried on its own. Scattered through the log are `reached max_pools` lines:
emma runs enough parallel downloads to overwhelm the Windows DNS resolver, which
then starts failing lookups.

**It is incremental.** Just re-run `emma`; it skips what it already cached and
retries only the failures, with far less parallelism pressure. The second run
took it from 161 failures to 2.

### Two failures that don't block the build

The last stragglers both failed with `reached max_pools`, and neither targets
`src/external include/`:

| Module | Target | Affects |
|---|---|---|
| `Editing\CES_Geometry 3.0` | `bin_x64\PowerDirector\CESdlls` | geometry/transform effects at **runtime** |
| `PowerDirector\WMProfileMgr 12.1` | `bin_x64\PowerDirector\runtime\ProfileMgr` | Windows Media output profiles in **Produce** |

These affect *running* the app, not *linking* it. Mop them up with another
`emma` run whenever convenient.

### Requirements emma assumes

- Internal network access to `ecl.cyberlink.com`
- An eModule login. The token lives in the **Windows credential store**, so it
  belongs to the account that authorised it. If emma prints
  `Please visit: https://ecl.cyberlink.com/connect/authorize?...` and then
  downloads nothing, that is the real problem. `emma login <token>` accepts a
  token non-interactively.
- Disk. The cache under `%USERPROFILE%\.emma\cache` reaches tens of GB.
  `emma purge` clears it.

### PowerShell has no `&&`

The session's terminal is PowerShell 5.1:

```
cd /d/PDR/pdr-win-ui-videoeditor && python emma_move_files.py
The token '&&' is not a valid statement separator in this version.
```

Use `;`, or two lines. The Bash tool in this repo's tooling takes POSIX syntax;
the PowerShell one does not.

---

## Reference: what else a fresh machine needs

From [`docs/unit-testing/RUNNER-SETUP.md`](../docs/unit-testing/RUNNER-SETUP.md):

| Tool | Verify with |
|---|---|
| Visual Studio 2022 + C++ workload + MFC v143 | VS Installer |
| Python 3.x | `python --version` |
| eModule import libraries | `dir "src\external include\x64\*.lib"` → 11 files |
| OpenCppCoverage | bundled portable in `tools\external\` |

`bootstrap.bat` handles VS Code extensions, PyYAML and the repo git hooks. It
does **not** provision eModule — that is the gap this document exists to fill.

```bash
help.bat status
```

...gives a checklist, but note it predates the eModule row and does not cover it.
