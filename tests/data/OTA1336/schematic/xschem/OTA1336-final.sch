v {xschem version=3.4.8RC file_version=1.3}
G {}
K {}
V {}
S {}
F {}
E {}
P 4 5 920 -1240 920 -1420 1900 -1420 1900 -1240 920 -1240 {}
P 4 5 1300 -1070 1300 -1210 1630 -1210 1630 -1070 1300 -1070 {}
P 4 6 1200 -505 1200 -745 1600 -745 1600 -445 1200 -445 1200 -505 {}
P 4 6 1625 -505 1625 -745 1835 -745 1835 -445 1625 -445 1625 -505 {}
T {Each rectangle means a tap ring. The ptap1/ntap1 devices model 
a corresponding contact resistance of each ring.} 190 -1660 0 0 0.4 0.4 {}
T {The bias current is supposed to be 80uA
} 190 -1540 0 0 0.4 0.4 {}
N 1340 -670 1460 -670 {
lab=#net1}
N 1010 -1195 1010 -1130 {
lab=iout}
N 1105 -1290 1660 -1290 {
lab=iout}
N 1700 -1000 1740 -1000 {
lab=vout}
N 1700 -820 1700 -730 {
lab=vout}
N 1500 -610 1700 -610 {
lab=vss}
N 1235 -1340 1400 -1340 {lab=vdd}
N 1290 -610 1400 -610 {lab=vss}
N 810 -1020 1010 -1020 {lab=iout}
N 1835 -1300 1835 -1265 {
lab=vdd}
N 1835 -1385 1835 -1360 {
lab=well}
N 1560 -1200 1560 -1170 {
lab=vdd}
N 1400 -525 1400 -490 {
lab=sub!}
N 1400 -610 1400 -585 {
lab=vss}
N 1700 -525 1700 -490 {
lab=sub!}
N 1700 -610 1700 -585 {
lab=vss}
N 1500 -700 1660 -700 {
lab=#net2}
N 1450 -940 1450 -820 {lab=#net2}
N 1500 -700 1500 -690 {lab=#net2}
N 1700 -1000 1700 -820 {
lab=vout}
N 1500 -820 1500 -700 {lab=#net2}
N 1630 -820 1700 -820 {lab=vout}
N 1500 -820 1570 -820 {lab=#net2}
N 1450 -820 1500 -820 {lab=#net2}
N 1290 -820 1350 -820 {lab=#net1}
N 1290 -720 1290 -700 {lab=#net1}
N 1290 -720 1340 -720 {lab=#net1}
N 1290 -820 1290 -720 {lab=#net1}
N 1340 -720 1340 -670 {lab=#net1}
N 1330 -670 1340 -670 {
lab=#net1}
N 1260 -990 1310 -990 {lab=v-}
N 1500 -990 1540 -990 {lab=v+}
N 1540 -990 1540 -890 {lab=v+}
N 810 -890 1540 -890 {lab=v+}
N 810 -950 1260 -950 {lab=v-}
N 1260 -990 1260 -950 {lab=v-}
N 1010 -1130 1010 -1020 {lab=iout}
N 1400 -1100 1400 -1050 {lab=#net3}
N 1010 -1130 1360 -1130 {lab=iout}
N 1700 -1260 1700 -1000 {lab=vout}
N 1400 -1340 1400 -1160 {lab=vdd}
N 1400 -1130 1480 -1130 {lab=#net4}
N 1480 -1130 1480 -1090 {lab=#net4}
N 1480 -1090 1560 -1090 {lab=#net4}
N 1560 -1110 1560 -1090 {lab=#net4}
N 1700 -1340 1700 -1320 {lab=vdd}
N 1010 -1340 1010 -1320 {lab=vdd}
N 810 -1340 1010 -1340 {lab=vdd}
N 1700 -1290 1760 -1290 {lab=well}
N 970 -1290 1010 -1290 {lab=well}
N 1290 -640 1290 -610 {lab=vss}
N 810 -610 1290 -610 {lab=vss}
N 1500 -640 1500 -610 {lab=vss}
N 1400 -610 1500 -610 {
lab=vss}
N 1500 -670 1530 -670 {lab=sub!}
N 1250 -670 1290 -670 {lab=sub!}
N 1700 -670 1700 -610 {lab=vss}
N 1700 -700 1740 -700 {lab=sub!}
N 1010 -1195 1105 -1195 {lab=iout}
N 1010 -1260 1010 -1195 {
lab=iout}
N 1105 -1290 1105 -1195 {lab=iout}
N 1050 -1290 1105 -1290 {
lab=iout}
N 1400 -1340 1700 -1340 {
lab=vdd}
N 1350 -940 1350 -820 {lab=#net1}
N 1235 -1050 1350 -1050 {lab=vdd}
N 1235 -1340 1235 -1050 {lab=vdd}
N 1010 -1340 1235 -1340 {lab=vdd}
C {sg13g2_pr/sg13_lv_nmos.sym} 1480 -670 0 0 {name=M4
l=9.75u
w=720n
ng=1
m=1
mm_ok=0
model=sg13_lv_nmos
spiceprefix=X
}
C {sg13g2_pr/sg13_lv_nmos.sym} 1310 -670 0 1 {name=M3
l=9.75u
w=720n
ng=1
m=1
mm_ok=0
model=sg13_lv_nmos
spiceprefix=X
}
C {sg13g2_pr/sg13_lv_pmos.sym} 1380 -1130 0 0 {name=M5
l=1.95u
w=5.3u
ng=1
m=1
mm_ok=0
model=sg13_lv_pmos
spiceprefix=X
}
C {sg13g2_pr/sg13_lv_pmos.sym} 1680 -1290 0 0 {name=M7
l=2.08u
w=75u
ng=8
m=1
mm_ok=0
model=sg13_lv_pmos
spiceprefix=X
}
C {sg13g2_pr/sg13_lv_nmos.sym} 1680 -700 0 0 {name=M6
l=9.75u
w=28.8u
ng=4
m=1
mm_ok=0
model=sg13_lv_nmos
spiceprefix=X
}
C {iopin.sym} 810 -950 0 1 {name=p10 lab=v-}
C {iopin.sym} 810 -890 2 0 {name=p11 lab=v+}
C {iopin.sym} 810 -610 0 1 {name=p5 lab=vss}
C {iopin.sym} 810 -1340 0 1 {name=p1 lab=vdd}
C {iopin.sym} 810 -1020 0 1 {name=p3 lab=iout}
C {iopin.sym} 1740 -1000 0 0 {name=p8 lab=vout}
C {sg13g2_pr/sg13_lv_pmos.sym} 1030 -1290 0 1 {name=M9
l=2.08u
w=75u
ng=8
m=1
mm_ok=0
model=sg13_lv_pmos
spiceprefix=X
}
C {sg13g2_pr/cap_cmim.sym} 1600 -820 3 0 {name=C2
model=cap_cmim
w=22.295e-6
l=22.295e-6
m=1
spiceprefix=X}
C {title-3.sym} 0 0 0 0 {name=l1 author="IHP-GmbH 2026" title="Miller OTA" rev=1.0 lock=true}
C {lab_pin.sym} 1835 -1265 0 0 {name=p35 sig_type=std_logic lab=vdd}
C {lab_pin.sym} 1835 -1385 0 1 {name=p36 sig_type=std_logic lab=well}
C {sg13g2_pr/ntap1.sym} 1835 -1330 2 0 {name=R6
model=ntap1
spiceprefix=X
w=0.725812e-6
l=89.914188e-6
}
C {lab_pin.sym} 1560 -1190 0 0 {name=p51 sig_type=std_logic lab=vdd}
C {sg13g2_pr/ptap1.sym} 1400 -555 0 0 {name=R5
model=ptap1
spiceprefix=X
w=0.735425e-6
l=30.304575e-6
}
C {lab_pin.sym} 1400 -490 0 1 {name=p34 sig_type=std_logic lab=sub!}
C {sg13g2_pr/ptap1.sym} 1700 -555 0 0 {name=R7
model=ptap1
spiceprefix=X
w=0.321008e-6
l=101.888992e-6
}
C {lab_pin.sym} 1700 -490 0 1 {name=p44 sig_type=std_logic lab=sub!}
C {lab_pin.sym} 1760 -1290 0 1 {name=p2 sig_type=std_logic lab=well}
C {lab_pin.sym} 970 -1290 2 1 {name=p4 sig_type=std_logic lab=well}
C {lab_pin.sym} 1530 -670 0 1 {name=p6 sig_type=std_logic lab=sub!}
C {lab_pin.sym} 1250 -670 2 1 {name=p7 sig_type=std_logic lab=sub!}
C {lab_pin.sym} 1740 -700 0 1 {name=p9 sig_type=std_logic lab=sub!}
C {sg13g2_pr/ntap1.sym} 1560 -1140 0 0 {name=R1
model=ntap1
spiceprefix=X
w=0.739264e-6
l=27.630736e-6
}
C {diff_pair.sym} 1400 -990 0 0 {name=x1}
