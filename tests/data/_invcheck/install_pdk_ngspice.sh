#!/usr/bin/env bash
set -euo pipefail
export PDK_ROOT=/home/adatsuk/IHP-Open-PDK
export PDK=ihp-sg13g2
cd "$PDK_ROOT/$PDK/libs.tech/ngspice"
python3 install.py
