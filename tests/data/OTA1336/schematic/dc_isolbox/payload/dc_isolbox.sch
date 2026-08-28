v {xschem version=Schematic 25.1.2 file_version=1.2}
G {}
K {schematic.header=<Qucs Schematic 25.1.2> schematic.view=<View=-150,-264,3002,1198,0.898321,25,221> schematic.view=<Grid=10,10,1> schematic.view=<DataSet=dc_isolbox.dat> schematic.view=<DataDisplay=dc_isolbox.dpl> schematic.view=<OpenDisplay=0> schematic.view=<Script=diode.m> schematic.view=<RunScript=0> schematic.view=<showFrame=3> schematic.view=<FrameText0=DC simulation of  ESD diodes> schematic.view=<FrameText1=Drawn By:IHP PDK Authors> schematic.view=<FrameText2=Date:2024> schematic.view=<FrameText3=Revision:1>}
V {}
S {}
E {}
N 120 600 300 600 {lab=}
N 300 600 300 600 {lab=isosub_net}
N 300 600 300 660 {lab=}
N 120 600 120 720 {lab=}
N 300 780 300 850 {lab=}
N 240 720 300 720 {lab=nwell_net}
N 120 780 120 850 {lab=}
C {qucs_blackbox.sym} 130 50 0 0 {name=INCLSCR1 symname=INCLSCR qucs.type=INCLSCR rotate=0 mirror=0}
C {isource.sym} 120 750 0 0 {name=I1 value=1 mA qucs.type=Idc rotate=1 mirror=0}
C {gnd.sym} 300 850 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {qucs_directive.sym} 90 280 0 0 {name=SW1 symname=.SW qucs.type=.SW rotate=0 mirror=0}
C {qucs_directive.sym} 90 170 0 0 {name=DC1 symname=.DC qucs.type=.DC rotate=0 mirror=0}
C {gnd.sym} 120 850 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {isolbox.sym} 300 750 0 0 {name=isolbox1 qucs.type=Lib qucs.model=isolbox rotate=0 mirror=0}
C {qucs_blackbox.sym} 310 170 0 0 {name=SpicePar1 symname=SpicePar qucs.type=SpicePar rotate=0 mirror=0}
C {qucs_directive.sym} 270 280 0 0 {name=SW2 symname=.SW qucs.type=.SW rotate=0 mirror=0}
