# Import external views (File → Import)

LibMan can convert schematics and layouts from other EDA formats into **CORE** view files and register them in the current project.

| Menu | Description |
|------|-------------|
| **File → Import...** | Open the import dialog |
| **File → Export...** | Export selected CORE views to GDS, Xschem, or Qucs |

Import requires a **CORE-enabled** build (`CONFIG+=no_core` disables this menu).

---

## Dialog fields

| Field | Description |
|-------|-------------|
| **Source format** | `GDS`, `Xschem`, `Qucs`, or `OAS` |
| **Target library** | Library from the open `.projects` file (same names as in [Project Editor](PROJECT_EDITOR.md)) |
| **Single file / Folder** | Import one file or every matching file in a directory |
| **Path** | Source file or folder (Browse...) |
| **Log** | Per-file success or failure after **Import** (updates progressively) |
| **Overwrite existing cell views** | Replace `*.core` files that already exist for the same cell/view |

The target library defaults to the library currently selected in the main window.

---

## What happens on import

For each source file:

1. LibMan creates `\<library-root>/\<cell>/\<cell>.\<view>.core` (or copies a native `.oas` file when no converter is available).
2. A **CORE converter** is run as an external process.
3. The new view is registered in LibMan (`LIBRARY_<lib>/<cell>/<view>`).
4. If a project file is open, **File → Save** runs automatically — a new `define("library", "path");` line is appended to `.projects`.

Cell name is taken from the source file base name (`OTA1336.sch` → cell `OTA1336`).

| Format | Source extensions | Converter | Result view |
|--------|-------------------|-----------|-------------|
| GDS | `.gds`, `.gds2` | `gds_to_core --all-cells` | `layout` |
| Xschem | `.sch`, `.sym` | `xschem_to_core` | `schematic` or `symbol` |
| Qucs | `.sch` | `qucs_to_core` | `schematic` |
| OAS | `.oas`, `.oas.gz` | `oas_to_core` if present, else native copy | `layout` or `oas` |

GDS import uses `--all-cells` so multi-cell libraries are preserved in one `.layout.core` file per source GDS.

---

## Converter tools (next to `libman.exe`)

LibMan does not embed converters in-process. On build, these tools from [CommonDB](https://github.com/IHP-GmbH/CommonDB) are copied next to the executable:

- `gds_to_core`, `core_to_gds`
- `xschem_to_core`, `core_to_xschem`
- `qucs_to_core`, `core_to_qucs`
- `oas_to_core` (when CORE is built with ZLIB)

**Search order** (`core/converter_paths.cpp`):

1. `LIBMAN_CONVERTER_DIR` environment variable (if set)
2. Directory containing `libman.exe`
3. `tools/` and `converters/` subdirectories next to the executable

Override example:

```powershell
$env:LIBMAN_CONVERTER_DIR = "C:\path\to\converters"
.\build\libman.exe
```

### Build integration

| Build system | Behaviour |
|--------------|-----------|
| **CMake** | `CORE_BUILD_EXAMPLES=ON` in `cmake/FetchCore.cmake`; POST_BUILD copies converters to `$<TARGET_FILE_DIR:libman>` |
| **qmake** | `scripts/fetch_core.cmd` / `fetch_core_linux.sh` build converter targets; `core_converter_deploy.pri` copies them after link |

After changing CORE or fetch scripts, delete `.deps/core-build/libman_core_built.stamp` (qmake) or reconfigure CMake so converters are rebuilt.

---

## Export (File → Export...)

Export CORE views from the current project to external formats:

| Format | Result extensions | Converter |
|--------|-------------------|-----------|
| GDS | `.gds` | `core_to_gds` |
| Xschem | `.sch` / `.sym` | `core_to_xschem` |
| Qucs | `.sch` | `core_to_qucs` |

Pick a destination folder and one or more `*.core` view files (or use the project tree selection). Converters are resolved the same way as import (`LIBMAN_CONVERTER_DIR`, directory next to `libman.exe`).

---

## Project file format

Imported views appear like any other view in `.projects`:

```text
define("my_lib", "my_lib/inv/inv.schematic.core");
define("my_lib", "my_lib/ota/ota.layout.core");
```

Use **[Project Editor](PROJECT_EDITOR.md)** (`Ctrl+E`) to review or edit entries. See also [CORE integration](CORE_INTEGRATION.md) for view naming (`*.schematic.core`, `*.layout.core`, …).

---

## Typical workflow

1. **File → Open...** — load `mydesign.projects`.
2. **File → Import...** — choose format, target library, folder of `.sch` files.
3. Check the log; failed rows stay in the log with a reason (converter missing, cell already exists, etc.).
4. Open schematic views (double-click) — on Windows use [Xschem + WSL](XSCHEM_INTEGRATION.md).

---

## Troubleshooting

| Symptom | What to check |
|---------|----------------|
| `Converter 'gds_to_core' was not found` | Rebuild with CORE; confirm `gds_to_core.exe` is next to `libman.exe` or set `LIBMAN_CONVERTER_DIR` |
| `Cell view already exists` | Delete the existing cell folder or pick another library |
| Import menu disabled / error about `no_core` | Rebuild without `CONFIG+=no_core`; CI stub builds omit Import |
| OAS imports as `.oas` not `.layout.core` | `oas_to_core` was not built (ZLIB missing in CORE build); native OAS copy still works |
| Project file unchanged | Open a `.projects` file first; otherwise use **File → Save** after import |

More: [Troubleshooting](../reference/TROUBLESHOOTING.md).

---

## Related topics

- [CORE integration](CORE_INTEGRATION.md) — fetch/build CommonDB, view types
- [Project Editor](PROJECT_EDITOR.md) — edit `define()` entries
- [Xschem integration](XSCHEM_INTEGRATION.md) — open imported schematics on Windows
- [Build guide](../BUILD.md) — CMake and qmake
