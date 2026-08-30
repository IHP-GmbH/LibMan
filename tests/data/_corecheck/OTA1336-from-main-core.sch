v {xschem version=3.4.8RC file_version=1.3}
G {}
V {}
S {}
E {}
N 990 -1140 1290 -1140 {lab=iout}
N 1500 -1040 1500 -970 {lab=v+}
N 1030 -990 1030 -970 {lab=v+}
N 740 -990 1030 -990 {lab=v+}
N 1030 -1040 1030 -1020 {lab=v-}
N 1030 -1040 1180 -1040 {lab=v-}
N 740 -1020 1030 -1020 {lab=v-}
N 1430 -880 1430 -820 {lab=vss}
N 1630 -910 1630 -820 {lab=vss}
N 940 -1110 940 -1090 {lab=iout}
N 940 -1090 940 -1060 {lab=iout}
N 1430 -820 1630 -820 {lab=vss}
N 1470 -1040 1500 -1040 {lab=v+}
N 1330 -1090 1430 -1090 {lab=#net2}
N 1330 -1190 1330 -1140 {lab=vdd}
N 740 -820 1220 -820 {lab=vss}
N 1430 -1090 1430 -1070 {lab=#net2}
N 990 -1140 990 -1090 {lab=iout}
N 1030 -970 1500 -970 {lab=v+}
N 1220 -820 1430 -820 {lab=vss}
N 940 -1090 990 -1090 {lab=iout}
N 1220 -1090 1220 -1070 {lab=#net2}
N 1220 -1040 1430 -1040 {lab=vdd}
N 1220 -950 1330 -950 {lab=#net1}
N 1630 -1040 1630 -970 {lab=vout}
N 1330 -950 1330 -880 {lab=#net1}
N 1220 -950 1220 -910 {lab=#net1}
N 1260 -880 1330 -880 {lab=#net1}
N 1220 -1090 1330 -1090 {lab=#net2}
N 1630 -1190 1630 -1140 {lab=vdd}
N 980 -1140 990 -1140 {lab=iout}
N 940 -1190 1330 -1190 {lab=vdd}
N 1570 -1140 1590 -1140 {lab=iout}
N 1630 -1040 1670 -1040 {lab=vout}
N 740 -1190 940 -1190 {lab=vdd}
N 1220 -880 1220 -820 {lab=vss}
N 1630 -1110 1630 -1040 {lab=vout}
N 1430 -910 1530 -910 {lab=#net3}
N 1220 -1010 1220 -950 {lab=#net1}
N 1330 -1190 1630 -1190 {lab=vdd}
N 1330 -1110 1330 -1090 {lab=#net2}
N 1590 -970 1630 -970 {lab=vout}
N 1330 -880 1390 -880 {lab=#net1}
N 1530 -910 1590 -910 {lab=#net3}
N 740 -1060 940 -1060 {lab=iout}
N 940 -1190 940 -1140 {lab=vdd}
N 1430 -1010 1430 -910 {lab=#net3}
N 1530 -970 1530 -910 {lab=#net3}
N 1630 -970 1630 -940 {lab=vout}
T {Basic topology of the Miller OTA with proper sizing of the transistors
The schematic can be used for initial simulations.
The value of bias current is 80 uA} 700 -1450 0 0 0.4 0.4 {}
C {sg13g2_pr/sg13_lv_nmos.sym} 1410 -880 2 1 {name=M4 l=9.75u w=720n ng=1 m=1 mm_ok=1 model=sg13_lv_nmos spiceprefix=X}
C {sg13g2_pr/sg13_lv_nmos.sym} 1240 -880 2 0 {name=M3 l=9.75u w=720n ng=1 m=1 mm_ok=1 model=sg13_lv_nmos spiceprefix=X}
C {sg13g2_pr/sg13_lv_pmos.sym} 1200 -1040 0 0 {name=M1 l=3.7u w=3.64u ng=1 m=2 mm_ok=1 model=sg13_lv_pmos spiceprefix=X}
C {sg13g2_pr/sg13_lv_pmos.sym} 1450 -1040 0 1 {name=M2 l=3.7u w=3.64u ng=1 m=2 mm_ok=1 model=sg13_lv_pmos spiceprefix=X}
C {sg13g2_pr/sg13_lv_pmos.sym} 1310 -1140 0 0 {name=M5 l=1.95u w=5.3u ng=1 m=1 mm_ok=0 model=sg13_lv_pmos spiceprefix=X}
C {sg13g2_pr/sg13_lv_pmos.sym} 1610 -1140 0 0 {name=M7 l=2.08u w=75u ng=8 m=1 mm_ok=0 model=sg13_lv_pmos spiceprefix=X}
C {sg13g2_pr/sg13_lv_nmos.sym} 1610 -910 2 1 {name=M6 l=9.75u w=28.8u ng=4 m=1 mm_ok=0 model=sg13_lv_nmos spiceprefix=X}
C {iopin.sym} 740 -1020 0 1 {name=p10 lab=v-}
C {iopin.sym} 740 -990 2 0 {name=p11 lab=v+}
C {iopin.sym} 740 -820 0 1 {name=p5 lab=vss}
C {iopin.sym} 740 -1190 0 1 {name=p1 lab=vdd}
C {iopin.sym} 740 -1060 0 1 {name=p3 lab=iout}
C {iopin.sym} 1670 -1040 0 0 {name=p8 lab=vout}
C {lab_pin.sym} 1570 -1140 0 0 {name=p6 sig_type=std_logic lab=iout}
C {sg13g2_pr/sg13_lv_pmos.sym} 960 -1140 0 1 {name=M9 l=2.08u w=75u ng=8 m=1 mm_ok=0 model=sg13_lv_pmos spiceprefix=X}
C {sg13g2_pr/cap_cmim.sym} 1560 -970 3 0 {name=C2 model=cap_cmim w=22.295e-6 l=22.295e-6 m=1 spiceprefix=X}
C {lab_pin.sym} 1330 -1040 3 0 {name=p2 sig_type=std_logic lab=vdd}
C {title-3.sym} 0 0 0 0 {name=l1 author="IHP-GmbH title="Miller rev=1.0 lock=true}
