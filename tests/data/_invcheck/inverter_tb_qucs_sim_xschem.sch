v {xschem version=Schematic 24.4.1 file_version=1.2}
G {}
K {schematic.header="<Qucs Schematic 24.4.1>" schematic.view=<View=0,0,900,650,1,0,0> schematic.view=<Grid=10,10,1> schematic.view=<DataSet=inverter_tb.dat> schematic.view=<DataDisplay=inverter_tb.dpl> schematic.view=<OpenDisplay=1> schematic.view=<Script=inverter_tb.m> schematic.view=<RunScript=0> schematic.view=<showFrame=0>}
V {}
S {}
E {}
N 310 350 310 350 {lab=}
N 350 300 350 350 {lab=}
N 310 250 310 250 {lab=}
N 310 250 310 350 {lab=}
N 310 320 310 250 {lab=}
N 350 300 420 300 {lab=}
N 120 380 120 410 {lab=}
N 310 140 310 220 {lab=}
N 120 320 310 320 {lab=}
N 120 320 120 350 {lab=}
N 350 250 350 300 {lab=}
N 520 230 520 260 {lab=}
N 520 170 520 140 {lab=}
N 350 380 350 450 {lab=}
N 310 280 310 320 {lab=}
N 310 140 520 140 {lab=}
C {qucs_blackbox.sym} 80 40 0 0 {name=INCLSCR1 symname=INCLSCR qucs.type=INCLSCR rotate=0 mirror=0}
C {qucs_directive.sym} 80 140 0 0 {name=TR1 symname=.TR qucs.type=.TR rotate=0 mirror=0}
C {vsource.sym} 520 200 0 0 {name=Vdd value="1.2 V" qucs.type=Vdc rotate=1 mirror=0}
C {gnd.sym} 520 260 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {vsource.sym} 120 350 0 0 {name=Vin value=0 qucs.type=Vpulse rotate=1 mirror=0}
C {gnd.sym} 120 410 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {gnd.sym} 330 450 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {sg13_lv_pmos.sym} 330 250 2 1 {name=Xp qucs.type=Lib qucs.model=sg13_lv_pmos rotate=0 mirror=1}
C {sg13_lv_nmos.sym} 330 350 0 0 {name=Xn qucs.type=Lib qucs.model=sg13_lv_nmos rotate=0 mirror=0}
