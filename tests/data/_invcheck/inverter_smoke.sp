* Minimal IHP SG13G2 ngspice smoke test (batch)
.lib cornerMOSlv.lib mos_tt

Vdd vdd 0 1.2
Vin vin 0 PULSE(0 1.2 0.5u 10n 10n 1u 2u 1)

* CMOS inverter (W/L from Analog Academy inverter)
Xp vout vin vdd vdd sg13_lv_pmos w=2u l=0.45u
Xn vout vin 0 0 sg13_lv_nmos w=1u l=0.45u

.control
save all
tran 50n 2u
write /tmp/inverter_smoke.raw
echo "TRAN done"
.endc

.end
