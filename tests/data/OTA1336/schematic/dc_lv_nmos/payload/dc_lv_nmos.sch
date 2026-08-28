v {xschem version=Schematic 24.4.1 file_version=1.2}
G {}
K {schematic.header=<Qucs Schematic 24.4.1> schematic.view=<View=-4,33,1528,906,1.02638,0,0> schematic.view=<Grid=10,10,1> schematic.view=<DataSet=dc_lv_nmos.dat> schematic.view=<DataDisplay=dc_lv_nmos.dpl> schematic.view=<OpenDisplay=0> schematic.view=<Script=dc_lv_nmos.m> schematic.view=<RunScript=0> schematic.view=<showFrame=3> schematic.view=<FrameText0=DC simulation of a Low Voltage  N type MOS> schematic.view=<FrameText1=Drawn By:IHP PDK Authors> schematic.view=<FrameText2=Date:2024> schematic.view=<FrameText3=Revision:1>}
V {}
S {}
E {}
N 160 750 250 750 {lab=}
N 160 750 160 770 {lab=}
N 270 670 290 720 {lab=}
N 290 780 270 840 {lab=}
N 290 750 330 750 {lab=}
N 270 590 450 590 {lab=}
N 270 590 270 610 {lab=}
N 450 590 450 720 {lab=}
N 450 780 450 840 {lab=}
N 160 830 160 840 {lab=}
N 330 750 330 840 {lab=}
C {qucs_directive.sym} 90 280 0 0 {name=SW1 symname=.SW qucs.type=.SW rotate=0 mirror=0}
C {gnd.sym} 270 840 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {gnd.sym} 330 840 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {IProbe.sym} 270 640 3 0 {name=Pr1 qucs.type=IProbe rotate=3 mirror=0}
C {vsource.sym} 450 750 0 0 {name=V2 value=1 V qucs.type=Vdc rotate=1 mirror=0}
C {gnd.sym} 450 840 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {vsource.sym} 160 800 0 0 {name=V1 value=1 V qucs.type=Vdc rotate=1 mirror=0}
C {gnd.sym} 160 840 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {qucs_directive.sym} 260 280 0 0 {name=SW2 symname=.SW qucs.type=.SW rotate=0 mirror=0}
C {qucs_blackbox.sym} 120 90 0 0 {name=INCLSCR1 symname=INCLSCR qucs.type=INCLSCR rotate=0 mirror=0}
C {sg13_lv_nmos.sym} 270 750 0 0 {name=sg13_lv_nmos1 qucs.type=Lib qucs.model=sg13_lv_nmos rotate=0 mirror=0}
