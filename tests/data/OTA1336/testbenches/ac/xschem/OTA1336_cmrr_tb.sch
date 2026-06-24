v {xschem version=3.4.8RC file_version=1.3}
G {}
K {}
V {}
S {}
F {}
E {}
B 2 880 -1350 1680 -950 {flags=graph


ypos1=0
ypos2=2
divy=5
subdivy=4
unity=1
x1=0

divx=5
subdivx=8
xlabmag=1.0
ylabmag=1.0


dataset=-1
unitx=1
logx=1
logy=0
autoload=0







sim_type=ac

y2=-0.042
y1=-160
color="4 4"
node=ph(vout)
x2=7}
B 2 880 -1750 1680 -1350 {flags=graph
y1=-8.7
y2=71
ypos1=0
ypos2=2
divy=5
subdivy=1
unity=1
x1=0

divx=5
subdivx=8
xlabmag=1.0
ylabmag=1.0
dataset=-1
unitx=1
logx=1
logy=0
x2=7
sim_type=ac
hilight_wave=0
color="6 4"
node="\\"vout vp / db20()\\"
av"}
B 2 1680 -1750 2480 -1350 {flags=graph
y1=21
y2=67
ypos1=0
ypos2=2
divy=5
subdivy=1
unity=1
x1=0

divx=5
subdivx=8
xlabmag=1.0
ylabmag=1.0


dataset=-1
unitx=1
logx=1
logy=0
x2=7
color=4
node=cmrr}
B 2 1680 -1350 2480 -950 {flags=graph
y1=-21
y2=30
ypos1=0
ypos2=2
divy=5
subdivy=1
unity=1
x1=0

divx=5
subdivx=8
xlabmag=1.0
ylabmag=1.0


dataset=-1
unitx=1
logx=1
logy=0
x2=7
color=4
node="psrr_linear db20()"}
N 80 -1050 80 -1020 {
lab=vp}
N 310 -1050 310 -1020 {
lab=vdd}
N 310 -960 310 -940 {
lab=GND}
N 190 -940 310 -940 {
lab=GND}
N 270 -550 270 -530 {
lab=GND}
N 270 -660 270 -630 {
lab=vdd}
N 110 -620 180 -620 {
lab=vp}
N 110 -560 130 -560 {
lab=vm}
N 450 -530 450 -520 {
lab=GND}
N 130 -280 200 -280 {
lab=vm}
N 130 -220 130 -210 {
lab=GND}
N 190 -940 190 -920 {
lab=GND}
N 80 -940 190 -940 {
lab=GND}
N 80 -960 80 -940 {
lab=GND}
N 230 -400 230 -390 {
lab=GND}
N 1630 -640 1630 -600 {
lab=vdd}
N 1520 -590 1540 -590 {
lab=vp}
N 1520 -530 1540 -530 {
lab=vp}
N 1790 -500 1790 -480 {
lab=GND}
N 1590 -500 1590 -470 {
lab=#net1}
N 1590 -410 1590 -390 {
lab=GND}
N 830 -530 850 -530 {
lab=vm}
N 830 -200 830 -190 {
lab=GND}
N 1180 -560 1210 -560 {
lab=Vout2}
N 1180 -560 1180 -260 {
lab=Vout2}
N 900 -500 900 -470 {
lab=#net2}
N 900 -410 900 -390 {
lab=GND}
N 940 -730 1070 -730 {
lab=VDDac}
N 940 -730 940 -600 {
lab=VDDac}
N 1070 -670 1070 -650 {
lab=GND}
N 710 -590 850 -590 {
lab=#net3}
N 710 -530 710 -510 {
lab=GND}
N 230 -530 230 -460 {lab=#net4}
N 130 -560 180 -560 {lab=vm}
N 130 -560 130 -280 {lab=vm}
N 510 -590 540 -590 {lab=Vout}
N 1630 -520 1630 -470 {lab=GND}
N 1520 -560 1520 -530 {lab=vp}
N 1500 -560 1520 -560 {lab=vp}
N 1520 -590 1520 -560 {lab=vp}
N 1710 -560 1880 -560 {lab=Vout1}
N 1020 -560 1180 -560 {lab=Vout2}
N 940 -520 940 -470 {lab=GND}
N 1010 -260 1180 -260 {lab=Vout2}
N 830 -260 950 -260 {
lab=vm}
N 830 -530 830 -260 {lab=vm}
N 800 -530 830 -530 {
lab=vm}
N 260 -280 510 -280 {lab=Vout}
N 510 -590 510 -280 {lab=Vout}
N 350 -590 510 -590 {lab=Vout}
C {vsource.sym} 80 -990 0 0 {name=V1 value="DC 0.6 AC 1 0"
}
C {vsource.sym} 310 -990 0 0 {name=VDD value="DC 1.2"}
C {gnd.sym} 190 -920 0 0 {name=l1 lab=GND}
C {gnd.sym} 270 -530 0 0 {name=l2 lab=GND}
C {lab_pin.sym} 270 -660 0 0 {name=p1 sig_type=std_logic lab=vdd}
C {lab_pin.sym} 310 -1050 0 0 {name=p2 sig_type=std_logic lab=vdd}
C {lab_pin.sym} 80 -1050 0 0 {name=p3 sig_type=std_logic lab=vp}
C {lab_pin.sym} 110 -620 0 0 {name=p5 sig_type=std_logic lab=vp}
C {lab_pin.sym} 110 -560 0 0 {name=p6 sig_type=std_logic lab=vm}
C {isource.sym} 230 -430 0 0 {name=I0 value=80u}
C {gnd.sym} 230 -390 0 0 {name=l3 lab=GND}
C {capa.sym} 450 -560 0 0 {name=Cload
m=1
value=500f
footprint=1206
device="ceramic capacitor"}
C {gnd.sym} 450 -520 0 0 {name=l5 lab=GND}
C {devices/code_shown.sym} 10 -1730 0 0 {name=MODEL only_toplevel=false
format="tcleval( @value )"
value="
.lib cornerCAP.lib cap_typ
.lib cornerMOSlv.lib mos_tt
.lib cornerRES.lib res_typ
"}
C {devices/code_shown.sym} 20 -1610 0 0 {name=NGSPICE only_toplevel=false 
value="
.control
op
save all
print v(vout) v(vp) v(vm) v(vout1) v(vout2)
write tb_OTA_op.raw
.endc

.control
op
ac dec 100 1 10e6 
save all
let Av = db(v(vout) / v(vp))
let PSRR_linear = v(vout2)/v(VDDac)
let CMRR = db((v(vout)/v(vp))/(v(vout1)/v(vp)))
let phase = 180*cph(vout)/pi
write output_file.raw 
.endc
"}
C {ind.sym} 230 -280 1 0 {name=L6
m=1
value=4G
footprint=1206
device=inductor}
C {capa.sym} 130 -250 0 0 {name=C1
m=1
value=4G
footprint=1206
device="ceramic capacitor"}
C {gnd.sym} 130 -210 0 0 {name=l7 lab=GND}
C {launcher.sym} 920 -900 0 0 {name=h5
descr="load waves" 
tclcommand="xschem raw_read $netlist_dir/output_file.raw ac"
}
C {gnd.sym} 1630 -470 0 0 {name=l8 lab=GND}
C {lab_pin.sym} 1630 -640 0 0 {name=p4 sig_type=std_logic lab=vdd}
C {lab_pin.sym} 1500 -560 0 0 {name=p10 sig_type=std_logic lab=vp}
C {isource.sym} 1590 -440 0 0 {name=I1 value=80u}
C {gnd.sym} 1590 -390 0 0 {name=l9 lab=GND}
C {capa.sym} 1790 -530 0 0 {name=Cload1
m=1
value=500f
footprint=1206
device="ceramic capacitor"}
C {gnd.sym} 1790 -480 0 0 {name=l10 lab=GND}
C {gnd.sym} 940 -470 0 0 {name=l4 lab=GND}
C {lab_pin.sym} 800 -530 0 0 {name=p11 sig_type=std_logic lab=vm}
C {isource.sym} 900 -440 0 0 {name=I2 value=80u}
C {gnd.sym} 900 -390 0 0 {name=l11 lab=GND}
C {ind.sym} 980 -260 1 0 {name=L13
m=1
value=4G
footprint=1206
device=inductor}
C {capa.sym} 830 -230 0 0 {name=C2
m=1
value=4G
footprint=1206
device="ceramic capacitor"}
C {gnd.sym} 830 -190 0 0 {name=l14 lab=GND}
C {vsource.sym} 1070 -700 0 0 {name=V2 value="DC 1.2 AC 1 0"
}
C {gnd.sym} 1070 -650 0 0 {name=l15 lab=GND}
C {lab_pin.sym} 940 -730 0 0 {name=p8 sig_type=std_logic lab=VDDac}
C {vsource.sym} 710 -560 0 0 {name=V4 value="DC 0.6"
}
C {gnd.sym} 710 -510 0 0 {name=l12 lab=GND}
C {OTA1336.sym} 260 -590 0 0 {name=x1}
C {OTA1336.sym} 1620 -560 0 0 {name=x2}
C {OTA1336.sym} 930 -560 0 0 {name=x3}
C {title-3.sym} 0 0 0 0 {name=l16 author="IHP-GmbH 2026" title="AC simulations" rev=1.0 lock=true}
C {lab_pin.sym} 540 -590 2 0 {name=p7 sig_type=std_logic lab=Vout}
C {lab_pin.sym} 1210 -560 2 0 {name=p9 sig_type=std_logic lab=Vout2}
C {lab_pin.sym} 1880 -560 2 0 {name=p12 sig_type=std_logic lab=Vout1}
