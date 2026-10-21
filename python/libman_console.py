#!/usr/bin/env python3
"""Compat shim — prefer door_console.py (DOOR_CONSOLE_PY / LIBMAN_CONSOLE_PY)."""
from pathlib import Path
import runpy

runpy.run_path(str(Path(__file__).with_name("door_console.py")), run_name="__main__")
