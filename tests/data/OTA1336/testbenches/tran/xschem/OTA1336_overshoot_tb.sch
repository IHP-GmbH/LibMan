v {xschem version=3.4.8RC file_version=1.3}
G {}
K {}
V {}
S {}
F {}
E {}
B 2 500 -1730 1800 -1310 {flags=graph


ypos1=0
ypos2=2
divy=5
subdivy=4
unity=1
x1=0

divx=5
subdivx=4
xlabmag=1.0
ylabmag=1.0


dataset=-1
unitx=1
logx=0
logy=0
autoload=0







sim_type=tran

y2=0.66
y1=0.59
color="4 8"
node="vout1
vin1"
x2=5e-06}
B 2 500 -1310 1800 -890 {flags=graph


ypos1=0
ypos2=2
divy=5
subdivy=4
unity=1
x1=0

divx=5
subdivx=4
xlabmag=1.0
ylabmag=1.0


dataset=-1
unitx=1
logx=0
logy=0
autoload=0







sim_type=tran

y2=1.82
y1=0.52
color="4 8"
node="vout2
vin2"
x2=5e-06}
T {Small signal response
50 mV pulse input} 1840 -1550 0 0 0.4 0.4 {}
T {Large Signal Response} 1850 -1110 0 0 0.4 0.4 {}
N 570 -510 570 -480 {
lab=#net1}
N 900 -500 900 -470 {
lab=#net2}
N 900 -410 900 -390 {
lab=GND}
N 940 -710 940 -600 {
lab=vdd}
N 490 -500 490 -420 {
lab=GND}
N 1070 -560 1210 -560 {lab=vout1}
N 940 -520 940 -470 {lab=GND}
N 940 -730 940 -710 {
lab=vdd}
N 490 -710 490 -560 {lab=vdd}
N 810 -530 850 -530 {lab=vout1}
N 810 -530 810 -340 {lab=vout1}
N 810 -340 1070 -340 {lab=vout1}
N 1070 -560 1070 -340 {lab=vout1}
N 1020 -560 1070 -560 {lab=vout1}
N 570 -590 570 -560 {
lab=vin1}
N 570 -590 850 -590 {lab=vin1}
N 490 -710 940 -710 {lab=vdd}
N 570 -620 570 -590 {lab=vin1}
N 1360 -510 1360 -480 {
lab=GND}
N 1690 -500 1690 -470 {
lab=#net3}
N 1690 -410 1690 -390 {
lab=GND}
N 1860 -560 2000 -560 {lab=vout2}
N 1730 -520 1730 -470 {lab=GND}
N 1600 -530 1640 -530 {lab=vout2}
N 1600 -530 1600 -340 {lab=vout2}
N 1600 -340 1860 -340 {lab=vout2}
N 1860 -560 1860 -340 {lab=vout2}
N 1810 -560 1860 -560 {lab=vout2}
N 1360 -590 1360 -560 {
lab=vin2}
N 1360 -590 1640 -590 {lab=vin2}
N 1360 -620 1360 -590 {lab=vin2}
N 1730 -650 1730 -600 {lab=vdd}
C {devices/code_shown.sym} 10 -1730 0 0 {name=MODEL only_toplevel=false
format="tcleval( @value )"
value="
.lib cornerCAP.lib cap_typ
.lib cornerMOSlv.lib mos_tt
.lib cornerRES.lib res_typ
"}
C {devices/code_shown.sym} 10 -1590 0 0 {name=NGSPICE only_toplevel=false 
value="
.control
save all
tran 1n 5u
meas tran Vmax MAX v(vout1) from=750n to=1.5u
let ovshoot = $&vmax/0.65
echo $&ovshoot
write slew_rate.raw 

.endc
"}
C {launcher.sym} 580 -820 0 0 {name=h5
descr="load waves" 
tclcommand="xschem raw_read $netlist_dir/slew_rate.raw tran"
}
C {gnd.sym} 940 -470 0 0 {name=l4 lab=GND}
C {isource.sym} 900 -440 0 0 {name=I2 value=80u}
C {gnd.sym} 900 -390 0 0 {name=l11 lab=GND}
C {gnd.sym} 570 -420 0 0 {name=l14 lab=GND}
C {vsource.sym} 490 -530 0 0 {name=V2 value="DC 1.2"
}
C {gnd.sym} 490 -420 0 0 {name=l15 lab=GND}
C {lab_pin.sym} 940 -730 0 0 {name=p8 sig_type=std_logic lab=vdd}
C {OTA1336.sym} 930 -560 0 0 {name=x3}
C {title-3.sym} 0 0 0 0 {name=l16 author="IHP-GmbH 2026" title="Transien small and large signal simulations" rev=1.0 lock=true}
C {lab_pin.sym} 1210 -560 2 0 {name=p9 sig_type=std_logic lab=vout1}
C {vsource.sym} 570 -530 0 0 {name=V3 value="pulse(0 50m 0 100p 100p 500n 1u)"
}
C {lab_pin.sym} 570 -620 2 0 {name=p1 sig_type=std_logic lab=vin1}
C {vsource.sym} 570 -450 0 0 {name=V1 value="DC 0.6"
}
C {gnd.sym} 1730 -470 0 0 {name=l1 lab=GND}
C {isource.sym} 1690 -440 0 0 {name=I1 value=80u}
C {gnd.sym} 1690 -390 0 0 {name=l2 lab=GND}
C {gnd.sym} 1360 -480 0 0 {name=l3 lab=GND}
C {OTA1336.sym} 1720 -560 0 0 {name=x1}
C {lab_pin.sym} 2000 -560 2 0 {name=p2 sig_type=std_logic lab=vout2}
C {vsource.sym} 1360 -530 0 0 {name=V4 value="pulse(0 1.2 1u 100p 100p 500n 10u)"
}
C {lab_pin.sym} 1360 -620 2 0 {name=p3 sig_type=std_logic lab=vin2}
C {lab_pin.sym} 1730 -650 0 0 {name=p4 sig_type=std_logic lab=vdd}
