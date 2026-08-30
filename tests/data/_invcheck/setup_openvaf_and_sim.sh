#!/bin/bash
set -e
cd /tmp
wget -q https://fides.fe.uni-lj.si/openvaf/download/openvaf-reloaded-osdi_0.4-linux_x64.tar.gz -O openvaf.tar.gz
tar -xzf openvaf.tar.gz
bin=$(find . -maxdepth 3 -type f \( -name openvaf -o -name openvaf-r \) | head -1)
echo "Installing $bin"
install -m 755 "$bin" /usr/local/bin/openvaf
openvaf --help | head -2

export PDK_ROOT=/home/adatsuk/IHP-Open-PDK
export PDK=ihp-sg13g2
cd "$PDK_ROOT/$PDK/libs.tech/verilog-a"
source ./openvaf-compile-va.sh
ls -la "$PDK_ROOT/$PDK/libs.tech/ngspice/osdi/"

ngspice -b /mnt/c/Users/anton/Documents/LibMan/tests/data/_invcheck/inverter_smoke.sp 2>&1 | tail -25
ls -la /tmp/inverter_smoke.raw
