# Xschem integration (Windows + WSL)

LibMan runs on **Windows**. Xschem in this flow runs in **WSL** (Linux GUI via WSLg). LibMan double-clicks a schematic/symbol view → Windows launcher → WSL → Xschem with CORE integration.

Layout views use [KLayout](KLAYOUT_INTEGRATION.md) natively on Windows; schematic/symbol use this guide.

## Overview

```
LibMan (Windows)
  double-click schematic / symbol
    → open-xschem-wsl.bat
      → wsl bash open-xschem-wsl.sh <path>
        → xschem --rcfile <xschemrc>
          → core.tcl (coretcl.so)
            → open *.schematic.core / *.symbol.core
```

| View in LibMan | File suffix | Tool |
|----------------|-------------|------|
| `schematic` | `*.schematic.core` | Xschem (WSL) |
| `symbol` | `*.symbol.core` | Xschem (WSL) |
| `layout` | `*.layout.core` | KLayout (Windows) |

## Prerequisites

### Windows

- LibMan built and running
- [WSL2](https://learn.microsoft.com/en-us/windows/wsl/install) with a Linux distro (Ubuntu recommended)
- **WSLg** (GUI support — default on Windows 11; on Windows 10 install compatible WSL + GUI stack)

### WSL

Inside WSL (from your **Xschem** repo checkout):

1. **Xschem** built and on `PATH` (`xschem` command works in a WSL terminal)
2. **coretcl.so** built for WSL (from CommonDB):

   ```bash
   cd /path/to/Xschem
   bash build-core-tcl.sh
   ```

   Expects **CommonDB** as a sibling directory (`../CommonDB`) unless `COMMONDB_ROOT` is set. Produces `integrations/coretcl.so` and `CommonDB/build-wsl/.../coretcl.so`.

3. Test manually:

   ```bash
   xschem --rcfile /path/to/Xschem/xschemrc
   ```

### Recommended directory layout

```text
parent/
  Xschem/          ← this repo (launchers, xschemrc, core.tcl)
  CommonDB/        ← sibling checkout (coretcl.so build)
  LibMan/          ← optional; Windows app
```

Paths are resolved from script locations — no editing required when clones stay siblings.

## Launcher scripts (Windows → WSL)

Scripts live in the **Xschem** repo (not LibMan):

| File | Role |
|------|------|
| `Xschem/integrations/open-xschem-wsl.bat` | Windows entry point for LibMan Tool Manager |
| `Xschem/integrations/open-xschem-wsl.sh` | WSL wrapper: path conversion, env vars, starts `xschem` |

### Path resolution (no hardcoded checkout paths)

| Component | How the path is chosen |
|-----------|-------------------------|
| `open-xschem-wsl.bat` | `%~dp0` + `wsl --cd` → runs `./open-xschem-wsl.sh` in `integrations/` |
| `open-xschem-wsl.sh` | Parent of `integrations/` = **Xschem root** → `xschemrc` |
| `xschemrc` | `[info script]` = Xschem root; `../CommonDB` = **CORE root** |
| `core.tcl` | Same; searches `integrations/coretcl.so` and `CommonDB/build-wsl/...` |

Optional environment overrides (WSL / Tcl):

| Variable | Purpose |
|----------|---------|
| `XSCHEM_ROOT` / `XSCHEM_HOME` | Xschem repo root |
| `XSCHEM_RC` | Alternate `xschemrc` file |
| `COMMONDB_ROOT` / `CORE_ROOT` | CommonDB checkout (non-sibling layout) |
| `XSCHEM_SRC` | Source tree for `build-xschem.sh` (default: `Xschem/xschem-src`) |

LibMan Tool Manager still needs the **full Windows path** to `open-xschem-wsl.bat` on your machine (one-time setup in Settings → Tools).

### What the launcher does

1. LibMan passes a **Windows** path to the view file (e.g. `C:\...\sg13g2_stdcell.schematic.core`).
2. `.bat` calls WSL with that path as `%1`.
3. `.sh` converts `C:\...` → `/mnt/c/...` (`wslpath` or fallback).
4. By extension:
   - `*.schematic.core` / `*.symbol.core` → sets `XSCHEM_OPEN_CORE` → `core.tcl` opens via CORE API after idle
   - `*.sch` / `*.sym` → sets `XSCHEM_OPEN_FILE` → direct `xschem load`
5. Xschem starts with `--rcfile` pointing at project `xschemrc` (loads `core.tcl`).

Supported arguments: one file path only.

## LibMan Tool Manager setup

**Settings → Tools** (Tool Manager):

| Tab / field | Value |
|-------------|--------|
| Tool name | e.g. `schematic` (or your custom tool id) |
| **Executable** | Full path to `open-xschem-wsl.bat` |

Example (use your actual checkout path):

```text
D:\work\Xschem\integrations\open-xschem-wsl.bat
```

| Property | Typical value |
|----------|----------------|
| **Name(s)** for schematic tool | `schematic` (and optionally `symbol`) |

Do **not** put `schematic.core` in Name(s) — LibMan matches the **view name** (`schematic`, `symbol`), not the file suffix.

Default schematic-related views come from CORE file names (`schematic`, `symbol`, `layout`). Register `schematic` and `symbol` with the same Xschem launcher if both use WSL.

### Opening from LibMan

1. Project file contains e.g. `define("lib", ".../cell.schematic.core");` — see [Project Editor](PROJECT_EDITOR.md).
2. Select **Cell** → double-click **schematic** (or **symbol**).
3. LibMan runs the bat file with the absolute path to the `.core` file.

## CORE + Xschem behaviour

Authoritative storage is the **CORE file**, not the `.sch` in `payload/`:

| Step | Action |
|------|--------|
| Open | CORE → export cell → `payload/<cell>.sch` → Xschem loads `.sch` |
| Edit | User edits in Xschem |
| Save | Xschem save → import back into bound `.schematic.core` / `.symbol.core` |

Window/tab title shows the **CORE filename** (e.g. `cell.schematic.core`), not `payload/cell.sch`.

`xschemrc` sets `initial_geometry` so a corrupted `~/.xschem/geometry` (`1x1+...`) does not shrink the window to a tiny strip.

### File naming (CommonDB convention)

| View | CORE file |
|------|-----------|
| Schematic | `<cell>.schematic.core` |
| Symbol | `<cell>.symbol.core` |
| Layout | `<cell>.layout.core` |

## Project layout example

```text
sg13g2_stdcell/
  sg13g2_stdcell/
    sg13g2_stdcell.schematic.core   ← LibMan opens this
    sg13g2_stdcell.layout.core
    payload/
      sg13g2_stdcell.sch            ← Xschem editor cache (export/import)
```

## Troubleshooting

| Symptom | What to check |
|---------|----------------|
| LibMan says tool not configured | Tool Manager: schematic tool → `open-xschem-wsl.bat` |
| `xschemrc not found` | Run from Xschem repo layout; or set `XSCHEM_ROOT` / `XSCHEM_RC` |
| `coretcl.so not found` | Run `build-core-tcl.sh` in WSL; ensure `../CommonDB` exists or set `COMMONDB_ROOT` |
| Xschem window is a tiny title bar | Corrupt `~/.xschem/geometry` with `1x1+` lines — remove them or delete file; `initial_geometry` in `xschemrc` helps |
| Title shows `.sch` not `.core` | Update `integrations/core.tcl` (title hooks); restart Xschem |
| Blank schematic | CORE file empty or export failed — check Messages in LibMan / Xschem stderr in WSL terminal |
| WSL `file not found` | Path conversion: ensure LibMan passes existing file; test `wslpath -u 'C:\...'` |
| No GUI in WSL | WSLg / DISPLAY; run `xschem` manually in WSL first |

### Clean geometry cache (WSL)

```bash
grep -v '1x1+' ~/.xschem/geometry > ~/.xschem/geometry.tmp && mv ~/.xschem/geometry.tmp ~/.xschem/geometry
```

### Manual test (same as LibMan)

```powershell
D:\work\Xschem\integrations\open-xschem-wsl.bat ^
  D:\work\LibMan\tests\data\sg13g2_stdcell\sg13g2_stdcell\sg13g2_stdcell.schematic.core
```

## Related docs

- [CORE integration](CORE_INTEGRATION.md) — building CommonDB / CORE in LibMan
- [KLayout integration](KLAYOUT_INTEGRATION.md) — layout views on Windows
- [Project Editor](PROJECT_EDITOR.md) — adding `*.schematic.core` to the project file
- Xschem repo: `integrations/core.tcl`, `integrations/open-xschem-wsl.*`, `xschemrc`
