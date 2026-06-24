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
subdivy=8
unity=1
x1=0

divx=5
subdivx=8
xlabmag=1.0
ylabmag=1.0


dataset=-1
unitx=1
logx=1
logy=1
autoload=0







sim_type=ac

y2=-0.54
y1=-2.1
color=4
node=vout2
x2=8}
B 2 1680 -1350 2480 -950 {flags=graph
y1=10
y2=41
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
x2=8
color=6
node=psrr}
N 720 -510 720 -500 {
lab=GND}
N 900 -500 900 -470 {
lab=#net1}
N 900 -410 900 -390 {
lab=GND}
N 940 -710 940 -600 {
lab=VDDac}
N 550 -530 550 -510 {
lab=GND}
N 1070 -560 1210 -560 {lab=Vout2}
N 940 -520 940 -470 {lab=GND}
N 550 -710 940 -710 {lab=VDDac}
N 940 -730 940 -710 {
lab=VDDac}
N 550 -710 550 -670 {lab=VDDac}
N 550 -610 550 -590 {lab=#net2}
N 720 -590 850 -590 {lab=#net3}
N 810 -530 850 -530 {lab=Vout2}
N 810 -530 810 -340 {lab=Vout2}
N 810 -340 1070 -340 {lab=Vout2}
N 1070 -560 1070 -340 {lab=Vout2}
N 1020 -560 1070 -560 {lab=Vout2}
N 720 -590 720 -560 {
lab=#net3}
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
ac dec 100 1 10e7 
*tran 10u 10m
let a = (vddac/vout2)
let PSRR = db(a)
write output_file.raw 
quit
.endc
"}
C {launcher.sym} 920 -900 0 0 {name=h5
descr="load waves" 
tclcommand="xschem raw_read $netlist_dir/output_file.raw ac"
}
C {gnd.sym} 940 -470 0 0 {name=l4 lab=GND}
C {isource.sym} 900 -440 0 0 {name=I2 value=80u}
C {gnd.sym} 900 -390 0 0 {name=l11 lab=GND}
C {gnd.sym} 720 -500 0 0 {name=l14 lab=GND}
C {vsource.sym} 550 -560 0 0 {name=V2 value="DC 1.2"
}
C {gnd.sym} 550 -510 0 0 {name=l15 lab=GND}
C {lab_pin.sym} 940 -730 0 0 {name=p8 sig_type=std_logic lab=VDDac}
C {OTA1336.sym} 930 -560 0 0 {name=x3}
C {title-3.sym} 0 0 0 0 {name=l16 author="IHP-GmbH 2026" title="AC simulations" rev=1.0 lock=true}
C {lab_pin.sym} 1210 -560 2 0 {name=p9 sig_type=std_logic lab=Vout2}
C {vsource.sym} 550 -640 0 0 {name=V1 value="DC 0 AC 1 sin(0 10m 1k)"
}
C {vsource.sym} 720 -530 0 0 {name=V3 value="DC 0.6"
}
