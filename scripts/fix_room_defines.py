#!/usr/bin/env python3
from pathlib import Path

root = Path(r"C:\Users\anton\Documents\LibMan")
repls = [
    ("LIBMAN_NO_CORE", "LIBMAN_NO_ROOM"),
    ("CONFIG+=no_core", "CONFIG+=no_room"),
    ("CONFIG, no_core", "CONFIG, no_room"),
    ("contains(CONFIG, no_core)", "contains(CONFIG, no_room)"),
    ("!contains(CONFIG, no_core)", "!contains(CONFIG, no_room)"),
    ("make -j1 core_fetch", "make -j1 room_fetch"),
    ("LIBMAN_CORE_SOURCE_DIR", "LIBMAN_ROOM_SOURCE_DIR"),
    ("Checkout CORE (CommonDB)", "Checkout ROOM (CommonDB)"),
    ("Verify CORE (CommonDB)", "Verify ROOM (CommonDB)"),
    ("Verify LIBMAN_NO_CORE", "Verify LIBMAN_NO_ROOM"),
    ("Build CORE", "Build ROOM"),
    ("no CORE)", "no ROOM)"),
]
skip = {".git", "build", ".deps"}
n = 0
for p in root.rglob("*"):
    if not p.is_file():
        continue
    if any(x in p.parts for x in skip):
        continue
    if p.suffix.lower() not in {
        ".cpp", ".h", ".hpp", ".pro", ".pri", ".yml", ".md", ".cmake", ".txt", ".cmd", ".sh",
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
print("libman", n)

wf = Path(r"C:\Users\anton\Documents\CommonDB\.github\workflows\build.yml")
t = wf.read_text(encoding="utf-8")
u = t.replace("core-build-ubuntu", "room-build-ubuntu").replace("core-coverage-html", "room-coverage-html")
wf.write_text(u, encoding="utf-8", newline="\n")

enc = Path(r"C:\Users\anton\Documents\CommonDB\scripts\fix_docs_encoding.py")
t = enc.read_text(encoding="utf-8")
enc.write_text(t.replace("Common Open Repository for EDA", "Reusable Open Object Model"), encoding="utf-8", newline="\n")
print("done")
