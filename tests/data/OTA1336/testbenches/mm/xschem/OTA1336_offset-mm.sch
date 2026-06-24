v {xschem version=3.4.8RC file_version=1.3}
G {}
K {}
V {}
S {}
F {}
E {}
B 2 820 -1750 2435 -1020 {flags=graph
y1=0.085
y2=1.2
ypos1=0
ypos2=2
divy=5
subdivy=1
unity=1
x1=-0.0011034928

divx=5
subdivx=4
xlabmag=1.0
ylabmag=1.0
dataset=-1
unitx=1
logx=0
logy=0
x2=0.001711257
sim_type=dc
hilight_wave=-1
color="6 4 4"
node="vp
vm
vout"
legend=1
digital=0
rainbow=1
autoload=1}
N 760 -655 760 -625 {
lab=vdd}
N 760 -565 760 -545 {
lab=GND}
N 1350 -685 1350 -655 {
lab=#net1}
N 1350 -595 1350 -545 {
lab=GND}
N 1550 -745 1660 -745 {lab=Vout}
N 1390 -705 1390 -545 {lab=GND}
N 990 -585 990 -545 {lab=GND}
N 990 -655 990 -625 {lab=#net2}
N 1130 -715 1300 -715 {lab=vm}
N 860 -655 990 -655 {lab=#net2}
N 1130 -735 1130 -715 {lab=vm}
N 1030 -715 1130 -715 {lab=vm}
N 1130 -775 1300 -775 {lab=vp}
N 1030 -655 1030 -635 {lab=#net3}
N 920 -775 920 -715 {lab=vp}
N 1390 -545 1550 -545 {lab=GND}
N 1230 -545 1350 -545 {lab=GND}
N 1350 -545 1390 -545 {lab=GND}
N 1030 -575 1030 -545 {lab=GND}
N 990 -545 1030 -545 {lab=GND}
N 860 -545 990 -545 {lab=GND}
N 860 -655 860 -625 {lab=#net2}
N 860 -565 860 -545 {lab=GND}
N 1550 -745 1550 -675 {lab=Vout}
N 1470 -745 1550 -745 {lab=Vout}
N 1550 -615 1550 -545 {lab=GND}
N 1390 -825 1390 -785 {lab=vdd}
N 760 -545 860 -545 {lab=GND}
N 1230 -545 1230 -535 {lab=GND}
N 1030 -545 1230 -545 {lab=GND}
N 1130 -800 1130 -775 {lab=vp}
N 920 -775 1130 -775 {lab=vp}
C {vsource.sym} 760 -595 0 0 {name=VDD value="DC 1.2"}
C {lab_pin.sym} 760 -655 0 0 {name=p2 sig_type=std_logic lab=vdd}
C {devices/code_shown.sym} 10 -1730 0 0 {name=MODEL only_toplevel=false
format="tcleval( @value )"
value="
.lib cornerCAP.lib cap_typ
.lib cornerMOSlv.lib mos_tt_mismatch
.lib cornerRES.lib res_typ
"}
C {devices/code_shown.sym} 20 -1580 0 0 {name=NGSPICE only_toplevel=false 
value="
.control
  
  shell rm tb_offset_mm.raw
  let mc_runs = 50
  let run = 0
  set curplot=new          
  set scratch=$curplot    
  set appendwrite
  let voff=unitvec(mc_runs)
  
  dowhile run < mc_runs   
    reset
    dc V1 -50m 50m 10u
    meas dc Vin_at when Vout=0.6	    
    set run = $&run   
    set plt = $curplot          
    setplot $scratch       
    let vvout\{$run\} = \{$plt\}.v(Vout)
    let voff[run]=\{$plt\}.Vin_at
    setplot $plt
    let run = run + 1
    write tb_offset_mm.raw
    end  
print mean(\{$scratch\}.voff)
print stddev(\{$scratch\}.voff)
wrdata offset.csv \{$scratch\}.voff
.endc
"}
C {launcher.sym} 885 -980 0 0 {name=h5
descr="load waves" 
tclcommand="xschem raw_read $netlist_dir/tb_offset_mm.raw dc"
}
C {gnd.sym} 1230 -535 0 0 {name=l4 lab=GND}
C {isource.sym} 1350 -625 0 0 {name=I2 value=80u}
C {lab_pin.sym} 1390 -825 1 0 {name=p8 sig_type=std_logic lab=vdd}
C {vsource.sym} 860 -595 0 0 {name=V1 value="DC 0.0"
}
C {OTA1336.sym} 1380 -745 0 0 {name=x3}
C {title-3.sym} 0 0 0 0 {name=l16 author="IHP-GmbH 2026" title="AC simulations" rev=1.0 lock=true}
C {lab_pin.sym} 1660 -745 2 0 {name=p9 sig_type=std_logic lab=Vout}
C {capa.sym} 1550 -645 0 0 {name=Cload2
m=1
value=500f
footprint=1206
device="ceramic capacitor"}
C {lab_pin.sym} 1130 -800 0 0 {name=p1 sig_type=std_logic lab=vp}
C {lab_pin.sym} 1130 -735 0 0 {name=p3 sig_type=std_logic lab=vm}
C {vsource.sym} 920 -685 0 0 {name=V2 value="DC 0.6"
}
C {vsource.sym} 1030 -685 0 0 {name=V3 value="DC 0.6"
}
C {vcvs.sym} 1030 -605 0 0 {name=E1 value=-1}
