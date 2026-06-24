v {xschem version=3.4.8RC file_version=1.3}
G {}
K {}
V {}
S {}
F {}
E {}
B 2 860 -1380 1660 -980 {flags=graph
y1=0.1
y2=1.1
ypos1=0
ypos2=2
divy=5
subdivy=1
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
x2=0.0002
color="4 7"
node="vp
vm"}
N 80 -1050 80 -1020 {
lab=vp}
N 460 -1050 460 -1020 {
lab=vdd}
N 460 -960 460 -940 {
lab=GND}
N 190 -940 460 -940 {
lab=GND}
N 840 -620 840 -600 {
lab=GND}
N 840 -730 840 -700 {
lab=vdd}
N 680 -690 750 -690 {
lab=vp}
N 680 -630 700 -630 {
lab=vm}
N 1020 -600 1020 -590 {
lab=GND}
N 700 -350 1080 -350 {
lab=vm}
N 190 -940 190 -920 {
lab=GND}
N 80 -940 190 -940 {
lab=GND}
N 80 -960 80 -940 {
lab=GND}
N 800 -470 800 -460 {
lab=GND}
N 800 -600 800 -530 {lab=#net1}
N 700 -630 750 -630 {lab=vm}
N 700 -630 700 -350 {lab=vm}
N 1080 -660 1110 -660 {lab=vm}
N 1080 -660 1080 -350 {lab=vm}
N 920 -660 1080 -660 {lab=vm}
C {vsource.sym} 80 -990 0 0 {name=V1 value="DC 0.6 AC 1 0 pulse(0.1, 1.1, 0, 10n, 10n, 50u 100u)"
}
C {vsource.sym} 460 -990 0 0 {name=VDD value="DC 1.2"}
C {gnd.sym} 190 -920 0 0 {name=l1 lab=GND}
C {gnd.sym} 840 -600 0 0 {name=l2 lab=GND}
C {lab_pin.sym} 840 -730 0 0 {name=p1 sig_type=std_logic lab=vdd}
C {lab_pin.sym} 460 -1050 0 0 {name=p2 sig_type=std_logic lab=vdd}
C {lab_pin.sym} 80 -1050 0 0 {name=p3 sig_type=std_logic lab=vp}
C {lab_pin.sym} 680 -690 0 0 {name=p5 sig_type=std_logic lab=vp}
C {lab_pin.sym} 680 -630 0 0 {name=p6 sig_type=std_logic lab=vm}
C {isource.sym} 800 -500 0 0 {name=I0 value=80u}
C {gnd.sym} 800 -460 0 0 {name=l3 lab=GND}
C {capa.sym} 1020 -630 0 0 {name=Cload
m=1
value=500f
footprint=1206
device="ceramic capacitor"}
C {gnd.sym} 1020 -590 0 0 {name=l5 lab=GND}
C {devices/code_shown.sym} 10 -1730 0 0 {name=MODEL only_toplevel=false
format="tcleval( @value )"
value="
.lib cornerCAP.lib cap_typ
.lib cornerMOSlv.lib mos_tt
.lib cornerRES.lib res_typ
"}
C {devices/code_shown.sym} 10 -1640 0 0 {name=NGSPICE only_toplevel=false 
value="
.control
op
save all
print v(vout) v(vp) v(vm) v(vout1) v(vout2)
write tb_OTA_op.raw
.endc

.control
save all
tran 10n 200u
save all
write tran.raw 
.endc
"}
C {launcher.sym} 920 -900 0 0 {name=h5
descr="load waves" 
tclcommand="xschem raw_read $netlist_dir/tran.raw tran"
}
C {OTA1336.sym} 830 -660 0 0 {name=x1}
C {title-3.sym} 0 0 0 0 {name=l16 author="IHP-GmbH 2026" title="AC simulations" rev=1.0 lock=true}
