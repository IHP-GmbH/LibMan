# Python Command Line (CML)

LibMan embeds a Virtuoso-style **Command Line** under **Messages** for interactive [ROOM](https://github.com/IHP-GmbH/Room) Python (`import room`).

## UI

| Element | Role |
|---------|------|
| **Messages** | Log + echoed `>>>` commands and Python output |
| **Command Line** | Multiline editor for Python |
| **Window → Messages / Command Line** | Show or hide each panel (saved in `QSettings`) |

### Keys

| Key | Action |
|-----|--------|
| **Enter** | Run the buffer (or selection) |
| **Shift+Enter** | New line |
| **↑ / ↓** | Command history (on the first / last line of a multiline buffer) |

History keeps up to 200 distinct commands for the session.

## Startup

On launch LibMan starts a persistent Python process with `room` preloaded:

1. Finds `python` / `python3` (`LIBMAN_PYTHON` overrides the executable).
2. Prefers CommonDB’s package via `PYTHONPATH` (`LIBMAN_ROOM_PYTHON` or sibling `../CommonDB/python`).
3. Sets `ROOM_C_DLL` when a built `room_c` / `libroom_c` is found next to LibMan or under CommonDB `build-python/python`.

If `import room` fails, a red warning appears in Messages; install bindings from the Room repo (`pip install ./python`) or set the env vars above.

## Examples

```python
db = room.open(r"C:/path/to/cell.schematic.room")
print(db.cell_names())
cell = db.find_cell(db.cell_names()[0])
cxt = cell.content()          # sole view when the file has one content
print(cxt.view(), len(cxt.instances()))
```

Open **`.room` files**, not view folders (e.g. `*.emsetup` is a directory).

Paths are trimmed by ROOM’s Python API (leading/trailing spaces are ignored).

Full API: [ROOM Python docs](https://ihp-gmbh.github.io/Room/python.html) · [Cell](https://ihp-gmbh.github.io/Room/python_cell.html).

## Related

- [ROOM integration](ROOM_INTEGRATION.md) — fetch/build ROOM with LibMan
- [Import](IMPORT.md) — GDS/Xschem/Qucs/OAS → `.room`
