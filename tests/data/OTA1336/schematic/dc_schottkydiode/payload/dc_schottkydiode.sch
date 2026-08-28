v {xschem version=Schematic 25.1.2 file_version=1.2}
G {}
K {schematic.header=<Qucs Schematic 25.1.2> schematic.view=<View=-259,31,1305,797,0.585621,0,1> schematic.view=<Grid=10,10,1> schematic.view=<DataSet=dc_schottkydiode.dat> schematic.view=<DataDisplay=dc_schottkydiode.dpl> schematic.view=<OpenDisplay=0> schematic.view=<Script=dc_schottkydiode.m> schematic.view=<RunScript=0> schematic.view=<showFrame=1> schematic.view=<FrameText0=Schottky diode example> schematic.view=<FrameText1=Drawn By: IHP-Open-PDK Authors 2025> schematic.view=<FrameText2=Date:> schematic.view=<FrameText3=Revision:>}
V {}
S {}
E {}
N 450 690 450 690 {lab=sub!}
N 240 710 240 710 {lab=sub!}
N 80 690 80 700 {lab=}
N 80 690 130 690 {lab=}
N 240 690 240 710 {lab=}
N 190 690 240 690 {lab=}
N 320 530 320 540 {lab=}
N 350 580 450 580 {lab=}
N 450 580 450 690 {lab=}
N 320 600 320 690 {lab=}
N 80 530 80 570 {lab=}
N 80 630 80 690 {lab=}
N 80 530 320 530 {lab=vd}
C {gnd.sym} 320 690 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {isource.sym} 80 600 0 0 {name=I1 value=1 mA qucs.type=Idc rotate=1 mirror=0}
C {schottky_nbl1.sym} 320 570 0 0 {name=schottky_nbl2 qucs.type=Lib qucs.model=schottky_nbl1 rotate=0 mirror=0}
C {vsource.sym} 160 690 3 0 {name=V1 value=0 V qucs.type=Vdc rotate=0 mirror=0}
C {gnd.sym} 80 700 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {qucs_directive.sym} 100 300 0 0 {name=SW1 symname=.SW qucs.type=.SW rotate=0 mirror=0}
C {qucs_directive.sym} 270 300 0 0 {name=SW2 symname=.SW qucs.type=.SW rotate=0 mirror=0}
C {qucs_directive.sym} 100 220 0 0 {name=DC1 symname=.DC qucs.type=.DC rotate=0 mirror=0}
C {qucs_blackbox.sym} 120 120 0 0 {name=SpiceLib1 symname=dio_tt qucs.type=SpiceLib qucs.model=dio_tt rotate=0 mirror=0}
C {qucs_blackbox.sym} 330 120 0 0 {name=SpicePar1 symname=SpicePar qucs.type=SpicePar rotate=0 mirror=0}
