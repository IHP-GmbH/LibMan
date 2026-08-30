#!/usr/bin/env bash
set -euo pipefail

export PDK_ROOT="${PDK_ROOT:-/home/adatsuk/IHP-Open-PDK}"
export PDK="${PDK:-ihp-sg13g2}"
export PATH="$HOME/.local/bin:$PATH"

SIM_DIR="/tmp/inverter_tb_sim"
rm -rf "$SIM_DIR"
mkdir -p "$SIM_DIR"

TB_DIR="/mnt/c/Users/anton/Documents/IHP-AnalogAcademy/modules/module_0_foundations/inverter"
cp "$TB_DIR/inverter_tb.sch" "$TB_DIR/inverter.sym" "$TB_DIR/inverter.sch" "$SIM_DIR/"

cd "$SIM_DIR"
export netlist_dir="$SIM_DIR"

echo "==> Netlisting with xschem..."
xschem -n -q -s -o "$SIM_DIR" "$SIM_DIR/inverter_tb.sch" 2>&1 | tail -20

NETLIST="$SIM_DIR/inverter_tb.spice"
if [ ! -f "$NETLIST" ]; then
  NETLIST=$(ls "$SIM_DIR"/*.spice "$SIM_DIR"/*.cir 2>/dev/null | head -1)
fi
if [ -z "${NETLIST:-}" ] || [ ! -f "$NETLIST" ]; then
  echo "ERROR: netlist not found in $SIM_DIR" >&2
  ls -la "$SIM_DIR" >&2
  exit 1
fi

echo "==> Netlist: $NETLIST"
head -40 "$NETLIST"

echo "==> Running ngspice batch..."
ngspice -b "$NETLIST" 2>&1 | tail -40

RAW="$SIM_DIR/test_inverter.raw"
if [ -f "$RAW" ]; then
  echo "OK: simulation produced $RAW ($(wc -c < "$RAW") bytes)"
else
  echo "ERROR: $RAW not found" >&2
  ls -la "$SIM_DIR" >&2
  exit 1
fi
