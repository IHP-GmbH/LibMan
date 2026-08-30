# CORE (CommonDB) integration

LibMan can optionally link [CORE](https://github.com/IHP-GmbH/CommonDB). CommonDB is a **private** repository. At configure time LibMan probes GitHub API access to `IHP-GmbH/CommonDB` (`curl` + `LIBMAN_CORE_GIT_TOKEN` / `GITHUB_TOKEN` if set). If the repo is reachable — CORE is enabled; otherwise stub implementations are used (`LIBMAN_NO_CORE`).

## Default behaviour

**qmake** — `core_build_config.pri` runs `scripts/probe_core_access.sh` (or `.cmd` on Windows):

```bash
mkdir -p build && cd build
qmake ../libman.pro
make -j1 capnp_install
make -j1 lstream_schemas
make -j"$(nproc)"
```

Without access you will see: `LibMan: building without CORE (CommonDB not available)`.

**CMake** — same probe via `cmake/ProbeCoreAccess.cmake`:

```bash
cmake -B build
cmake --build build -j
```

### Enable CORE access

| Method | When |
|--------|------|
| `export LIBMAN_CORE_GIT_TOKEN=ghp_...` | PAT with `repo` read on `IHP-GmbH/CommonDB` |
| `export GITHUB_TOKEN=...` | Same (fallback env var) |
| Clone to `.deps/CommonDB` | Local checkout (no probe needed) |
| `LIBMAN_CORE_SOURCE_DIR=/path/to/CommonDB` | Side-by-side development tree |

After a successful probe, qmake builds fetch CORE on `make core_fetch` (or automatically when the target exists in CI).

### Force overrides

| qmake | CMake |
|-------|-------|
| `CONFIG+=no_core` | `-DLIBMAN_FORCE_NO_CORE=ON` |
| `CONFIG+=core` | `-DLIBMAN_FORCE_CORE=ON` |

## Full CORE build (with access)

### qmake

```bash
export LIBMAN_CORE_GIT_TOKEN=ghp_...
qmake ../libman.pro
make -j1 capnp_install
make -j1 lstream_schemas
make -j1 core_fetch
make -j"$(nproc)"
```

### CMake

```bash
export LIBMAN_CORE_GIT_TOKEN=ghp_...
cmake -B build
cmake --build build -j
```

FetchContent clones CORE to `.deps/CommonDB/` and links `CORE::core` / `CORE::core_utils`.

Re-configure after changing the CORE revision:

```powershell
Remove-Item -Recurse -Force .deps/CommonDB
cmake -B build
```

Or pin a tag/commit:

```powershell
cmake -B build -DCORE_GIT_TAG=91705d7
```

## Local CORE checkout (development)

```powershell
cmake -B build -DLIBMAN_CORE_SOURCE_DIR=C:/path/to/CommonDB
```

## Installed CORE (advanced)

```powershell
cmake -B build -DLIBMAN_FETCH_CORE=OFF -DCORE_DIR=...
```

(`find_package(CORE)` — requires CORE installed with `cmake --install`.)

## CMake cache variables

| Variable | Default | Description |
|----------|---------|-------------|
| `LIBMAN_FORCE_CORE` | `OFF` | Enable CORE even if GitHub probe fails |
| `LIBMAN_FORCE_NO_CORE` | `OFF` | Disable CORE even if probe succeeds |
| `LIBMAN_FETCH_CORE` | `ON` | Fetch CORE from GitHub (when CORE enabled) |
| `CORE_GIT_URL` | `https://github.com/IHP-GmbH/CommonDB.git` | Repository URL |
| `CORE_GIT_TAG` | `main` | Branch, tag, or commit |
| `LIBMAN_CORE_SOURCE_DIR` | *(empty)* | Local tree instead of fetch |

## Using CORE in LibMan code

```cpp
#include "database.h"

core::Database db;
db.loadFromFile("layout.core");
```

Link targets are already set in `CMakeLists.txt` (`CORE::core`, `CORE::core_utils`).

## CI (GitHub Actions)

| Job | CORE |
|-----|------|
| `build-linux-no-core` | No token — probe fails, stubs only |
| `build-linux`, `tests-linux`, `build-windows`, `build-rhel8`, `build-ubuntu24` | `GH_PAT` (or legacy `LIBMAN_CORE_GIT_TOKEN`) — CommonDB checkout + full CORE |

Add repository secret (org-level `GH_PAT` is preferred — same token as Qucs/XSchem/KLayout CI):

| Secret | Description |
|--------|-------------|
| `GH_PAT` | PAT with `repo` read access to `IHP-GmbH/CommonDB` |
| `LIBMAN_CORE_GIT_TOKEN` | Legacy alias (still accepted if `GH_PAT` is unset) |

`qmake` / `cmake` auto-detect access; no manual `CONFIG+=core` required in CI.

## Layout view in LibMan

`layout` (and legacy `core`) is a first-class layout view suffix (like `gds`, `oas`, `lstr`):

- **Create:** View panel → New → Layout → `layout` (creates `<cell>/<cell>.layout.core`)
- **Tree:** expand `layout` to browse cell hierarchy from `LibIndex`
- **Open:** double-click opens the file in KLayout with a resolved top cell; see **[KLayout integration](KLAYOUT_INTEGRATION.md)** for server setup, root-cell rules, and mcore plugin notes.

**Schematic/symbol (`*.schematic.core`, `*.symbol.core`):** on Windows open in **Xschem via WSL** ([Xschem integration](XSCHEM_INTEGRATION.md)) and/or **Qucs-S** ([Qucs-S integration](QUCS_INTEGRATION.md)) — register one or both in Tool Manager.

Default `LayoutViews` property: `gds,oas,lstr,layout`.

## Project file and views

View paths come from the project file (`define(library, path)`). Edit entries in **[Project Editor](PROJECT_EDITOR.md)** (**File → Edit Project...**, `Ctrl+E`). Bulk-import from external formats via **[Import](IMPORT.md)** (**File → Import...**). After Save or import, LibMan reloads libraries; removed views disappear from the tree and documentation for the selected library is refreshed.

## Converter tools

With `CORE_BUILD_EXAMPLES=ON` (default in LibMan CMake / qmake fetch scripts), converter executables are deployed next to `libman.exe` for the Import dialog. See **[Import](IMPORT.md)** for formats, folder import, and `LIBMAN_CONVERTER_DIR`.
