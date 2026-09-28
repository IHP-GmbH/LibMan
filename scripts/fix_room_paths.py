#!/usr/bin/env python3
from pathlib import Path

root = Path(r"C:\Users\anton\Documents\LibMan")
skip = {".git", "build", ".deps"}
repls = [
    ("core/core_path_utils", "room/room_path_utils"),
    ("core/room_path_utils", "room/room_path_utils"),
    ("core/room_file_lock", "room/room_file_lock"),
    ("core/converter_paths", "room/converter_paths"),
    ("core/libman_no_core_stubs", "room/libman_no_room_stubs"),
    ("core/libman_no_room_stubs", "room/libman_no_room_stubs"),
    ("core/roomcellreader", "room/roomcellreader"),
    ("core/coreReadAsync", "room/roomReadAsync"),
    ("core/roomReadAsync", "room/roomReadAsync"),
    ("core/roomKlayoutBridge", "room/roomKlayoutBridge"),
    ("core_path_utils.h", "room_path_utils.h"),
    ("core_path_utils.cpp", "room_path_utils.cpp"),
    ("coreReadAsync", "roomReadAsync"),
    ("libman_no_core_stubs", "libman_no_room_stubs"),
    ("core_import_service", "room_import_service"),
    ("tst_core_path_utils", "tst_room_path_utils"),
    (" $$PWD/../core/", " $$PWD/../room/"),
    ("$$PWD/../core/", "$$PWD/../room/"),
    ("\n    core/", "\n    room/"),
    ("\n        core/", "\n        room/"),
    (" list(APPEND SOURCES core/", " list(APPEND SOURCES room/"),
    ("SOURCES += core/", "SOURCES += room/"),
]

n = 0
for p in root.rglob("*"):
    if not p.is_file():
        continue
    if any(x in p.parts for x in skip) or any(str(x).startswith("build") for x in p.parts):
        continue
    if p.suffix.lower() not in {
        ".cpp", ".h", ".hpp", ".pro", ".pri", ".cmake", ".md", ".yml", ".txt", ".json",
    } and p.name != "CMakeLists.txt":
        continue
    try:
        t = p.read_text(encoding="utf-8")
    except Exception:
        continue
    u = t
    for a, b in repls:
        u = u.replace(a, b)
    if u != t:
        p.write_text(u, encoding="utf-8", newline="\n")
        n += 1
        print(p.relative_to(root))
print("files", n)

tests = root / "tests"
for old, new in [
    ("tst_core_path_utils.cpp", "tst_room_path_utils.cpp"),
    ("tst_core_path_utils.h", "tst_room_path_utils.h"),
]:
    src, dst = tests / old, tests / new
    if src.exists():
        src.rename(dst)
        print("renamed", old)
