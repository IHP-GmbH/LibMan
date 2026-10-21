#!/usr/bin/env python3
"""DOOR Command Line driver ??? persistent ROOM REPL.

Loaded by DOOR at startup (not meant to be run as __main__ alone).
Exposes ``room``, ``load``, ``define``, and ``undefine`` in the session namespace.
"""

from __future__ import annotations

import json
import os
import sys
import traceback

g = {"__name__": "__main__"}

try:
    import room

    g["room"] = room
except Exception as e:
    print("WARNING: import room failed: %s" % e, file=sys.stderr)


def load(path):
    """SKILL-style: run a whole file into the persistent Command Line namespace."""
    exec(open(path, encoding="utf-8-sig").read(), g)


def _emit(cmd, **payload):
    """Ask LibMan (parent process) to update project UI / .projects without reload dialog."""
    msg = {"cmd": cmd}
    msg.update(payload)
    # One line; LibMan strips these before showing Messages.
    sys.stdout.write("@@LIBMAN_JSON@@" + json.dumps(msg, ensure_ascii=False) + "\n")
    sys.stdout.flush()


def define(library, path):
    """
    Register an existing view file with the open LibMan project.

    Writes a define() into the current .projects (escaped) and updates the
    library/cell/view lists in-place ??? no reload prompt.
    """
    abs_path = os.path.abspath(os.path.expanduser(str(path)))
    _emit("define", library=str(library), path=abs_path)
    return abs_path


def undefine(library, path):
    """Remove a view registration from the open LibMan project (does not delete the file)."""
    abs_path = os.path.abspath(os.path.expanduser(str(path)))
    _emit("undefine", library=str(library), path=abs_path)
    return abs_path


g["load"] = load
g["define"] = define
g["undefine"] = undefine

sys.stdout.flush()
sys.stderr.flush()

while True:
    header = sys.stdin.readline()
    if not header:
        break
    header = header.strip()
    if not header:
        continue
    try:
        nbytes = int(header)
    except ValueError:
        print("ERROR: bad command header: %r" % (header,), file=sys.stderr)
        print("@@LIBMAN_DONE@@")
        sys.stdout.flush()
        sys.stderr.flush()
        continue
    buf = sys.stdin.read(nbytes)
    if len(buf) < nbytes:
        break
    src = buf if isinstance(buf, str) else buf.decode("utf-8", "replace")
    try:
        try:
            code = compile(src, "<libman>", "eval")
        except SyntaxError:
            code = None
        if code is not None:
            result = eval(code, g, g)
            if result is not None:
                print(repr(result))
        else:
            exec(compile(src, "<libman>", "exec"), g, g)
    except SystemExit:
        raise
    except BaseException:
        traceback.print_exc()
    print("@@LIBMAN_DONE@@")
    sys.stdout.flush()
    sys.stderr.flush()
