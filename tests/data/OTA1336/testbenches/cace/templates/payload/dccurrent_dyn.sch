v {xschem version=3.4.8RC file_version=1.3}
G {}
V {}
S {}
E {}
N 730 -990 1370 -990 {lab=inm}
N 610 -1160 1490 -1160 {lab=vdd}
N 1490 -1160 1490 -1070 {lab=vdd}
N 1370 -1000 1370 -990 {lab=inm}
N 730 -990 730 -670 {lab=inm}
N 730 -1070 1370 -1070 {lab=inp}
N 1450 -970 1450 -940 {lab=#net1}
N 840 -940 1450 -940 {lab=#net1}
N 610 -1160 610 -670 {lab=vdd}
N 1370 -1000 1400 -1000 {lab=inm}
N 1700 -970 1880 -970 {lab=VSUB}
N 1370 -1070 1370 -1060 {lab=inp}
N 1370 -1060 1400 -1060 {lab=inp}
N 1060 -900 1060 -670 {lab=vss}
N 1060 -900 1490 -900 {lab=vss}
N 590 -610 1060 -610 {lab=VSUB}
N 840 -940 840 -670 {lab=#net1}
N 1490 -990 1490 -900 {lab=vss}
N 730 -1070 730 -1050 {lab=inp}
N 1570 -1030 1900 -1030 {lab=out}
T {Based on a template made by R. Timothy Edwards
November 27, 2023
Revision 0
Open sourced under Apache 2.0 license} 1950 -1670 0 0 0.4 0.4 {}
C {devices/vsource.sym} 730 -640 0 0 {name=VVcm value="DC savecurrent=false}
C {devices/lab_pin.sym} 590 -610 0 0 {name=p1 sig_type=std_logic lab=VSUB}
C {devices/vsource.sym} 1060 -640 0 0 {name=Vvss value="DC savecurrent=false}
C {devices/lab_wire.sym} 1210 -1160 0 1 {name=p11 sig_type=std_logic lab=vdd}
C {devices/lab_wire.sym} 1210 -900 0 1 {name=p24 sig_type=std_logic lab=vss}
C {devices/lab_wire.sym} 1900 -1030 0 0 {name=p25 sig_type=std_logic lab=out}
C {devices/res.sym} 1700 -1000 0 0 {name=Rout value=CACE\{Rout\} device=resistor}
C {devices/capa.sym} 1840 -1000 0 0 {name=Cout value=CACE\{Cout\}}
C {devices/lab_pin.sym} 1880 -970 0 1 {name=p27 sig_type=std_logic lab=VSUB}
C {devices/vsource.sym} 610 -640 0 0 {name=Vvdd value="DC savecurrent=false}
C {devices/code_shown.sym} 60 -1350 0 0 {name=CONTROL only_toplevel=false value=".control idd=$&Irms}
C {devices/res.sym} 910 -580 0 0 {name=RSUB value=0.01 device=resistor}
C {devices/gnd.sym} 910 -550 0 0 {name=l1 lab=GND}
C {devices/lab_pin.sym} 730 -1070 0 0 {name=p2 sig_type=std_logic lab=inp}
C {devices/vsource.sym} 730 -1020 0 0 {name=VVdiff value="DC savecurrent=false}
C {devices/lab_pin.sym} 730 -930 0 0 {name=p3 sig_type=std_logic lab=inm}
C {OTA1336.sym} 1480 -1030 0 0 {name=x1}
C {isource.sym} 840 -640 0 0 {name=Ibias value="DC}
C {devices/code_shown.sym} 50 -1650 0 0 {name=SETUP only_toplevel=false value="* TEMP=CACE\{temperature\} warn=1}
C {title-3.sym} 0 0 0 0 {name=l2 author="IHP-GmbH title="CACE rev=1.0 lock=true}
