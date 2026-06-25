v {xschem version=3.4.8RC file_version=1.3}
G {}
V {}
S {}
E {}
N 1450 -1070 1450 -1060 {lab=inp}
N 1450 -1020 1450 -1010 {lab=inm}
N 1450 -780 1450 -620 {lab=VSUB}
N 1020 -950 1020 -680 {lab=#net1}
N 730 -620 1450 -620 {lab=VSUB}
N 1650 -1040 1770 -1040 {lab=out}
N 870 -1080 1450 -1080 {lab=inp}
N 1020 -950 1530 -950 {lab=#net1}
N 750 -1170 750 -680 {lab=vdd}
N 1770 -1040 2040 -1040 {lab=out}
N 1840 -980 2020 -980 {lab=VSUB}
N 1580 -840 1770 -840 {lab=out}
N 750 -1170 1570 -1170 {lab=vdd}
N 1200 -910 1200 -680 {lab=vss}
N 1200 -910 1570 -910 {lab=vss}
N 1570 -1170 1570 -1080 {lab=vdd}
N 870 -1080 870 -680 {lab=inp}
N 1450 -1070 1480 -1070 {lab=inp}
N 1450 -840 1520 -840 {lab=inm}
N 1450 -1080 1450 -1070 {lab=inp}
N 1450 -1010 1480 -1010 {lab=inm}
N 1530 -980 1530 -950 {lab=#net1}
N 1450 -1010 1450 -840 {lab=inm}
N 1770 -1040 1770 -840 {lab=out}
N 1570 -1000 1570 -910 {lab=vss}
T {Based on a template made by R. Timothy Edwards
November 27, 2023
Revision 0
Open sourced under Apache 2.0 license} 2030 -1670 0 0 0.4 0.4 {}
C {devices/lab_pin.sym} 730 -620 0 0 {name=p1 sig_type=std_logic lab=VSUB}
C {devices/vsource.sym} 1200 -650 0 0 {name=Vvss value="DC savecurrent=false}
C {devices/lab_wire.sym} 1350 -1170 0 1 {name=p11 sig_type=std_logic lab=vdd}
C {devices/lab_wire.sym} 1350 -910 0 1 {name=p24 sig_type=std_logic lab=vss}
C {devices/lab_wire.sym} 2040 -1040 0 0 {name=p25 sig_type=std_logic lab=out}
C {devices/res.sym} 1840 -1010 0 0 {name=Rout value=CACE\{Rout\} device=resistor}
C {devices/capa.sym} 1980 -1010 0 0 {name=Cout value=CACE\{Cout\}}
C {devices/lab_pin.sym} 2020 -980 0 1 {name=p27 sig_type=std_logic lab=VSUB}
C {devices/vsource.sym} 750 -650 0 0 {name=Vvdd value="DC savecurrent=false}
C {devices/code_shown.sym} 40 -1340 0 0 {name=CONTROL only_toplevel=false value=".control =}
C {devices/res.sym} 1050 -590 0 0 {name=RSUB value=0.01 device=resistor}
C {devices/gnd.sym} 1050 -560 0 0 {name=l1 lab=GND}
C {devices/lab_wire.sym} 1450 -1020 0 0 {name=p4 sig_type=std_logic lab=inm}
C {OTA1336.sym} 1560 -1040 0 0 {name=x1}
C {isource.sym} 1020 -650 0 0 {name=Ibias value="DC}
C {devices/code_shown.sym} 20 -1680 0 0 {name=SETUP only_toplevel=false value="* TEMP=CACE\{temperature\} warn=1}
C {title-3.sym} 0 0 0 0 {name=l2 author="IHP-GmbH title="CACE rev=1.0 lock=true}
C {vsource.sym} 870 -650 0 0 {name=V1 value="DC}
C {ind.sym} 1550 -840 1 0 {name=L6 m=1 value=4G footprint=1206 device=inductor}
C {capa.sym} 1450 -810 0 0 {name=C1 m=1 value=4G footprint=1206 device="ceramic}
C {devices/lab_wire.sym} 1450 -1060 2 1 {name=p2 sig_type=std_logic lab=inp}
