# KLayout integration

LibMan opens layout views (`gds`, `oas`, `lstr`, `layout` / `*.layout.core`) in [KLayout](https://www.klayout.de/) via a persistent **KLayout server** — a background KLayout instance that polls a JSON command file.

## Tool Manager setup

| Property | Typical value |
|----------|----------------|
| **Layout** tool | Path to `klayout` (or `klayout.exe`) |
| **LayoutViews** | `gds,oas,lstr,layout` (default includes `core` alias for layout CORE) |

Schematic/symbol views use a separate tool (e.g. Xschem via WSL); see [Xschem integration](XSCHEM_INTEGRATION.md).

## Opening layouts

### Double-click on the layout view root

Double-click the **view node** itself (`gds`, `oas`, `lstr`, or `layout`) — not a cell inside the expanded hierarchy.

Double-click **opens KLayout** and does **not** expand or collapse the tree node (use the arrow/disclosure control or single-click expand for hierarchy browsing).

LibMan:

1. Resolves the file path sent to KLayout (GDS/OAS/LStream directly; `*.layout.core` natively when the [mcore](https://github.com/IHP-GmbH/Xschem/tree/main/integrations/klayout/mcore) streamer is installed).
2. Picks a **root cell** to activate (see [Root cell selection](#root-cell-selection)).
3. Starts KLayout with the server script if needed, then sends an `open` command with the file and cell name.
4. On a later double-click on the same layout root while KLayout is already running, sends a `select` command (file is not reloaded).

### Double-click on a hierarchy cell

Double-click a **child cell** under the layout view. LibMan always opens/selects that exact cell name.

## Root cell selection

When you open the layout **view root**, LibMan chooses the KLayout cell as follows:

| Priority | Rule | Example |
|----------|------|---------|
| 1 | LibMan **group name** (selected cell in the groups list) if that name exists in the layout file | Group `sg13g2_io`, GDS contains `sg13g2_io` → opens `sg13g2_io` |
| 2 | **Single top cell** in the file hierarchy | One wrapper cell `TOP` with children shown in the tree → opens `TOP` |
| 3 | **First top cell** (sorted) when several top-level cells exist | `sg13g2_stdcell.layout.core` with 84 stdcells, group `sg13g2_stdcell` not in file → opens first cell alphabetically (e.g. `sg13g2_and2_1`) |
| 4 | No matching cell | File opens without an explicit cell selection (KLayout default) |

Hierarchy is taken from the in-memory cache when the tree was expanded; otherwise LibMan reads it synchronously from the file (GDS/OAS/LStream/CORE parsers).

This matches the tree rules for CORE layout: a single wrapper top cell is shown as its children in the UI, but KLayout still activates the real top cell when you open the layout root.

## KLayout server protocol

Commands are written as JSON to a temp file (`libman_klayout_cmd_<pid>.json`), polled every 250 ms by an embedded Python script started with `klayout -rr <script>`.

| Action | Fields | Behaviour |
|--------|--------|-----------|
| `open` | `file`, `cell` | Load layout into a view (same-view mode); select `cell` if non-empty; zoom fit |
| `select` | `file`, `cell` | Switch to an already open view; select `cell`; zoom fit |

An `.alive` sidecar file reports the server PID for health checks.

## CORE layout files

`*.layout.core` files are passed **directly** to KLayout (no temporary GDS export) when the mcore plugin is available. Install the plugin from the Xschem/CommonDB integration package and register it in KLayout's `streamers` folder.

See also: [CORE integration](CORE_INTEGRATION.md).

## Troubleshooting

| Symptom | Check |
|---------|--------|
| KLayout does not start | **Layout** tool path in Tool Manager; executable exists |
| File opens but wrong/empty cell | Group name may not exist in file; expand layout tree once so hierarchy cache is warm |
| `*.layout.core` not recognized | mcore streamer not installed in KLayout |
| Second double-click does nothing | KLayout server died; close KLayout and double-click again to restart |
| Double-click expands tree instead of opening | Expected: only **child cells** expand; layout **root** opens KLayout. Use the disclosure arrow to expand |

More: [Troubleshooting](../reference/TROUBLESHOOTING.md).
