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







sim_type=noise

y2=-5
y1=-7.8
color="4 7"
node="inoise_spectrum
onoise_spectrum"
x2=8}
N 390 -570 390 -540 {
lab=vp}
N 620 -570 620 -540 {
lab=vdd}
N 620 -480 620 -460 {
lab=GND}
N 500 -460 620 -460 {
lab=GND}
N 500 -460 500 -440 {
lab=GND}
N 390 -460 500 -460 {
lab=GND}
N 390 -480 390 -460 {
lab=GND}
N 900 -500 900 -380 {
lab=#net1}
N 900 -320 900 -290 {
lab=GND}
N 940 -520 940 -290 {lab=GND}
N 830 -530 850 -530 {
lab=#net2}
N 940 -660 940 -600 {lab=vdd}
N 900 -290 940 -290 {lab=GND}
N 900 -290 900 -280 {lab=GND}
N 830 -530 830 -450 {lab=#net2}
N 1020 -560 1080 -560 {lab=Vout}
N 1140 -560 1280 -560 {lab=Vout}
N 820 -590 850 -590 {lab=vp}
N 1040 -290 1080 -290 {lab=GND}
N 1080 -340 1080 -290 {lab=GND}
N 1040 -330 1040 -290 {lab=GND}
N 940 -290 1040 -290 {lab=GND}
N 1040 -450 1040 -390 {lab=#net2}
N 830 -450 1040 -450 {lab=#net2}
N 1080 -560 1080 -380 {lab=Vout}
N 1140 -430 1140 -290 {lab=GND}
N 1080 -290 1140 -290 {lab=GND}
N 1140 -290 1280 -290 {lab=GND}
N 1280 -430 1280 -290 {lab=GND}
N 1280 -560 1280 -490 {lab=Vout}
N 1140 -560 1140 -490 {lab=Vout}
N 1080 -560 1140 -560 {lab=Vout}
N 1080 -580 1080 -560 {lab=Vout}
C {vsource.sym} 390 -510 0 0 {name=V1 value="DC 0.6 AC 1"
}
C {vsource.sym} 620 -510 0 0 {name=VDD value="DC 1.2"}
C {gnd.sym} 500 -440 0 0 {name=l1 lab=GND}
C {lab_pin.sym} 620 -570 0 0 {name=p2 sig_type=std_logic lab=vdd}
C {lab_pin.sym} 390 -570 0 0 {name=p3 sig_type=std_logic lab=vp}
C {devices/code_shown.sym} 10 -1730 0 0 {name=MODEL only_toplevel=false
format="tcleval( @value )"
value="
.lib cornerCAP.lib cap_typ
.lib cornerMOSlv.lib mos_tt
.lib cornerRES.lib res_typ
"}
C {devices/code_shown.sym} 10 -1620 0 0 {name=NGSPICE only_toplevel=false 
value="
.control
save all
noise V(Vout) V1 dec 10 1 1e7 
setplot noise1
write OTA1336_tb_noise.raw 
.endc
"}
C {launcher.sym} 920 -900 0 0 {name=h5
descr="load waves" 
tclcommand="xschem raw_read $netlist_dir/OTA1336_tb_noise.raw noise"
}
C {isource.sym} 900 -350 0 0 {name=I2 value=80u}
C {gnd.sym} 900 -280 0 0 {name=l14 lab=GND}
C {OTA1336.sym} 930 -560 0 0 {name=x3}
C {title-3.sym} 0 0 0 0 {name=l16 author="IHP-GmbH 2026" title="AC simulations" rev=1.0 lock=true}
C {lab_pin.sym} 1080 -580 2 0 {name=p9 sig_type=std_logic lab=Vout}
C {lab_pin.sym} 940 -660 0 0 {name=p1 sig_type=std_logic lab=vdd}
C {lab_pin.sym} 820 -590 0 0 {name=p4 sig_type=std_logic lab=vp}
C {devices/code_shown.sym} 10 -1450 0 0 {name=NGSPICE1 only_toplevel=false 
value="
.control
save all
noise V(Vout) V1 dec 10 1 1e7 
setplot noise2
unset sqrnoise
echo $&onoise_total
 
.endc
"}
C {devices/vcvs.sym} 1040 -360 0 1 {name=E1 value=1.0}
C {devices/res.sym} 1140 -460 0 0 {name=Rout
value=100000000.0
device=resistor}
C {devices/capa.sym} 1280 -460 0 0 {name=Cout
value=1e-13}
