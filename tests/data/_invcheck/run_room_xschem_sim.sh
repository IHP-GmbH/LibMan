#!/usr/bin/env bash
set -euo pipefail

export PDK_ROOT="${PDK_ROOT:-/home/adatsuk/IHP-Open-PDK}"
export PDK="${PDK:-ihp-sg13g2}"
export PATH="$HOME/.local/bin:/usr/bin:/bin:$PATH"

XSCHEM_ROOT="/mnt/c/Users/anton/Documents/XSchem-coredb"
SIM_DIR="/tmp/inv_core_sim"
TB_DIR="/mnt/c/Users/anton/Documents/IHP-AnalogAcademy/modules/module_0_foundations"
CORE_EXPORT="/mnt/c/Users/anton/Documents/LibMan/tests/data/_invcheck/_xschem_sim.sch"

rm -rf "$SIM_DIR"
mkdir -p "$SIM_DIR"
cp "$CORE_EXPORT" "$SIM_DIR/inverter_tb.sch"
cp "$TB_DIR/inverter/inverter.sym" "$SIM_DIR/"
cp "$TB_DIR/inverter/inverter.sch" "$SIM_DIR/"

cd "$SIM_DIR"
export netlist_dir="$SIM_DIR"
xschem --rcfile "$XSCHEM_ROOT/integrations/xschem-batch.rc" -n -q -s -o "$SIM_DIR" inverter_tb.sch

echo "==> NETLIST CHECK"
grep -E "Vin |Vdd |Vout |XM|MISSING|\.lib|\.control|tran" inverter_tb.spice || true

echo "==> NGSPICE"
ngspice -b inverter_tb.spice 2>&1 | tail -12

RAW=$(ls "$SIM_DIR"/*.raw 2>/dev/null | head -1 || true)
if [ -n "$RAW" ] && [ -f "$RAW" ]; then
  echo "OK: raw $RAW ($(wc -c < "$RAW") bytes)"
else
  echo "ERROR: no raw file" >&2
  exit 1
fi
