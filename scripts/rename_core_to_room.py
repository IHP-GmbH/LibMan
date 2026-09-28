#!/usr/bin/env python3
"""Rename CORE/core integration to ROOM/room in LibMan."""
from __future__ import annotations

import os
import re
import shutil
from pathlib import Path

ROOT = Path(r"C:\Users\anton\Documents\LibMan")
SKIP = {".git", "build", ".deps", "node_modules"}

REPL = [
    ("CORE::core_utils", "ROOM::room_utils"),
    ("CORE::core", "ROOM::room"),
    ("FetchCore.cmake", "FetchRoom.cmake"),
    ("FetchCore", "FetchRoom"),
    ("LIBMAN_CORE_SOURCE_DIR", "LIBMAN_ROOM_SOURCE_DIR"),
    ("CORE_INTEGRATION", "ROOM_INTEGRATION"),
    ("core_deps_finalize.pri", "room_deps_finalize.pri"),
    ("core_deps.pri", "room_deps.pri"),
    ("fetch_core_linux.sh", "fetch_room_linux.sh"),
    ("fetch_core.cmd", "fetch_room.cmd"),
    ("core_fetch", "room_fetch"),
    ("coreKlayoutBridge", "roomKlayoutBridge"),
    ("corecellreader", "roomcellreader"),
    ("CoreCellReader", "RoomCellReader"),
    ("core_file_lock", "room_file_lock"),
    ("core_export_service", "room_export_service"),
    ("tst_core_file_lock", "tst_room_file_lock"),
    ("run_core_xschem_sim", "run_room_xschem_sim"),
    ("legacy_import.core", "legacy_import.room"),
    ("coreLayoutPathForKLayout", "roomLayoutPathForKLayout"),
    ("loadCoreHierarchyAsync", "loadRoomHierarchyAsync"),
    ("namespace core", "namespace room"),
    ("core::", "room::"),
    (".schematic.core", ".schematic.room"),
    (".layout.core", ".layout.room"),
    (".symbol.core", ".symbol.room"),
    ("*.core", "*.room"),
    (".core\"", ".room\""),
    (".core'", ".room'"),
    (".core ", ".room "),
    (".core\n", ".room\n"),
    (".core)", ".room)"),
    (".core,", ".room,"),
    ("CORE ", "ROOM "),
    (" CORE", " ROOM"),
    ("`CORE`", "`ROOM`"),
    ("**CORE**", "**ROOM**"),
]

MOVES = [
    ("cmake/FetchCore.cmake", "cmake/FetchRoom.cmake"),
    ("core_deps.pri", "room_deps.pri"),
    ("core_deps_finalize.pri", "room_deps_finalize.pri"),
    ("scripts/fetch_core_linux.sh", "scripts/fetch_room_linux.sh"),
    ("scripts/fetch_core.cmd", "scripts/fetch_room.cmd"),
    ("docs/setup/CORE_INTEGRATION.md", "docs/setup/ROOM_INTEGRATION.md"),
    ("core/coreKlayoutBridge.cpp", "core/roomKlayoutBridge.cpp"),
    ("core/coreKlayoutBridge.h", "core/roomKlayoutBridge.h"),
    ("core/corecellreader.cpp", "core/roomcellreader.cpp"),
    ("core/corecellreader.h", "core/roomcellreader.h"),
    ("core/core_file_lock.cpp", "core/room_file_lock.cpp"),
    ("core/core_file_lock.h", "core/room_file_lock.h"),
    ("src/core_export_service.cpp", "src/room_export_service.cpp"),
    ("src/core_export_service.h", "src/room_export_service.h"),
    ("tests/tst_core_file_lock.cpp", "tests/tst_room_file_lock.cpp"),
    ("tests/data/_invcheck/run_core_xschem_sim.sh", "tests/data/_invcheck/run_room_xschem_sim.sh"),
    ("tests/data/_invcheck/_legacy_import.core", "tests/data/_invcheck/_legacy_import.room"),
]

DIR_MOVES = [
    ("core", "room"),
]


def main() -> None:
    if not ROOT.exists():
        print("LibMan not found")
        return

    for a, b in DIR_MOVES:
        old, new = ROOT / a, ROOT / b
        if old.exists() and not new.exists():
            shutil.move(str(old), str(new))
            print("dir", a, "->", b)

    for a, b in MOVES:
        # after core->room dir move, adjust paths that started with core/
        old_rel = a
        if a.startswith("core/") and (ROOT / "room").exists():
            old_rel = "room/" + a[len("core/"):]
        old, new = ROOT / old_rel, ROOT / b
        # also try original if dir move already applied in MOVES list order
        if not old.exists():
            old = ROOT / a
        if old.exists() and not new.exists():
            new.parent.mkdir(parents=True, exist_ok=True)
            shutil.move(str(old), str(new))
            print("move", old_rel, "->", b)

    n = 0
    for dirpath, dirnames, filenames in os.walk(ROOT):
        dirnames[:] = [d for d in dirnames if d not in SKIP and not d.startswith("build")]
        for name in filenames:
            path = Path(dirpath) / name
            if any(p in SKIP or str(p).startswith("build") for p in path.parts):
                continue
            if path.suffix.lower() not in {
                ".h", ".hpp", ".cpp", ".cc", ".pri", ".pro", ".cmake", ".md",
                ".yml", ".yaml", ".json", ".sh", ".cmd", ".bat", ".txt", ".sch",
                ".cir", ".gitignore",
            } and name not in {"CMakeLists.txt"}:
                continue
            try:
                text = path.read_text(encoding="utf-8")
            except Exception:
                try:
                    text = path.read_text(encoding="latin-1")
                except Exception:
                    continue
            new = text
            for old, rep in REPL:
                new = new.replace(old, rep)
            new = re.sub(r"(?<![\w:])core::", "room::", new)
            if new != text:
                path.write_text(new, encoding="utf-8", newline="\n")
                n += 1
                print("edit", path.relative_to(ROOT))
    print("edited", n)


if __name__ == "__main__":
    main()
