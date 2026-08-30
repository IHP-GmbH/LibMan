#!/usr/bin/env bash
export PATH="$HOME/.local/bin:$PATH"
export PDK_ROOT="${PDK_ROOT:-$HOME/IHP-Open-PDK}"
export PDK="${PDK:-ihp-sg13g2}"
rm -rf /tmp/sim-vout-test
mkdir -p /tmp/sim-vout-test
cp /tmp/libman-xschem-sim-7619/inverter_tb.sch /tmp/sim-vout-test/
cp /mnt/c/Users/anton/Documents/IHP-AnalogAcademy/modules/module_0_foundations/inverter/inverter.sym /tmp/sim-vout-test/
cp /mnt/c/Users/anton/Documents/IHP-AnalogAcademy/modules/module_0_foundations/inverter/inverter.sch /tmp/sim-vout-test/
cd /tmp/sim-vout-test
xschem -n -q -s -o /tmp/sim-vout-test --rcfile /mnt/c/Users/anton/Documents/XSchem-coredb/integrations/xschem-batch.rc inverter_tb.sch > /tmp/vout-netlist.log 2>&1
echo NETLIST_LOG
tail -10 /tmp/vout-netlist.log
echo SPICE_MOS
grep XM inverter_tb.spice || true
grep MISSING inverter_tb.spice || echo no_missing
ngspice -b inverter_tb.spice > /tmp/vout-ng.log 2>&1
echo NGSPICE
tail -15 /tmp/vout-ng.log
echo RAW
strings test_inverter.raw | head -16
