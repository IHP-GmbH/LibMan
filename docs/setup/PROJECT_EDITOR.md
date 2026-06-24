# Project Editor

LibMan stores design libraries in a **project file** (`.projects` or `.lib`). Each line is a `define(library, path)` entry that maps a logical library name to a view file (GDS, OAS, LStream, or CORE).

The **Project Editor** is a Cadence Library Path Editor–style table for editing those entries without hand-editing the file.

## Open the editor

| Menu | Shortcut |
|------|----------|
| **File → Edit Project...** | `Ctrl+E` |

The window title shows the current project path, for example:

`Project Editor: C:/.../sg13g2.projects`

An asterisk (`*`) means there are unsaved changes.

## Table columns

| Column | Meaning |
|--------|---------|
| **Library** | Logical library name (e.g. `ihp_sg13g2`) — same for many rows if one library has several cells/views |
| **Path** | View file path. Shown as an **absolute** path in the editor; written as **project-relative** on Save |

Each row corresponds to one `define("library", "path");` in the project file.

### Example project file

```text
define("ihp_sg13g2", "sg13g2_stdcell/sg13g2_stdcell/sg13g2_stdcell.gds");
define("ihp_sg13g2", "sg13g2_stdcell/sg13g2_stdcell/sg13g2_stdcell.layout.core");
define("ihp_sg13g2", "sg13g2_stdcell/sg13g2_stdcell/sg13g2_stdcell.schematic.core");
```

## Editing

- Type directly in **Library** and **Path** cells.
- An empty row at the bottom is always available for new entries.
- **Return** moves to the next field as usual in a table.

### Context menu (right-click)

| Action | Shortcut | Description |
|--------|----------|-------------|
| **Add Library...** | `Ctrl+Shift+I` | Inserts a new row below the selection |
| **Delete** | `Ctrl+Shift+D` | Removes selected row(s) |
| **Browse path...** | — | File picker for the view file |

**Double-click** the **Path** column to open the file browser as well.

### Browse path behaviour

- Inserts the **full absolute path** (e.g. `C:\Users\...\file.gds`) so paths are unambiguous in the table.
- If **Library** is empty, copies the library name from the **row above** (typical when adding another view to the same library).
- On **Save**, paths are stored relative to the `.projects` file directory when possible (portable projects).
- Both absolute and relative paths in the file are resolved correctly when LibMan loads the project.

## File menu (inside Project Editor)

| Action | Shortcut | Description |
|--------|----------|-------------|
| **Save** | `Ctrl+S` | Writes to the current project file and reloads LibMan |
| **Save As...** | — | Save under a new name, then reload |
| **Close** | `Ctrl+W` | Close (prompts if modified) |

Saving from the main window (**File → Save**, `Ctrl+S`) still works and writes the same `define(...)` list from the in-memory library registry.

## What happens on Save

1. Rows with **both** Library and Path filled are written to the project file.
2. Rows with a missing library name or path are skipped.
3. LibMan **reloads** the project:
   - Removed `define` entries disappear from the **View** tree (old `LIBRARY_*` keys are cleared first).
   - The previously selected **Project** and **Cell** are restored when possible.
   - **Documentation** and **categories** for the selected library are reloaded (PDF list is not left empty after save).

## Documentation panel

PDF files are **not** listed in the Project Editor. They are discovered automatically when a library is active:

- `doc/*.pdf` under the library root (derived from view paths), and
- `*.pdf` files anywhere under the project file directory (recursive).

After Project Editor Save, select the library in the main window if needed — documentation should repopulate automatically when the same library stays selected.

## Related topics

- [Import](IMPORT.md) — convert GDS, Xschem, Qucs, OAS into CORE views and add `define()` rows
- [KLayout integration](KLAYOUT_INTEGRATION.md) — opening layout views from the View tree
- [CORE integration](CORE_INTEGRATION.md) — `*.layout.core`, `*.schematic.core`
- [Xschem integration](XSCHEM_INTEGRATION.md) — WSL launchers, Tool Manager
- [Troubleshooting](../reference/TROUBLESHOOTING.md) — common Project Editor and view issues
