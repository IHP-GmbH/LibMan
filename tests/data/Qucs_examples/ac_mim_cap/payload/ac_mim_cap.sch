v {xschem version=Schematic 24.4.1 file_version=1.2}
G {}
K {schematic.header=<Qucs Schematic 24.4.1> schematic.view=<View=-143,-6,1617,996,0.893214,0,0> schematic.view=<Grid=10,10,1> schematic.view=<DataSet=ac_mim_cap.dat> schematic.view=<DataDisplay=ac_mim_cap.dpl> schematic.view=<OpenDisplay=0> schematic.view=<Script=cmim_AC.m> schematic.view=<RunScript=0> schematic.view=<showFrame=3> schematic.view=<FrameText0=AC Mim capacitor simulation> schematic.view=<FrameText1=Drawn By: IHP PDK Authors> schematic.view=<FrameText2=Date:2024> schematic.view=<FrameText3=Revision:1>}
V {}
S {}
E {}
N 360 780 360 780 {lab=Vout2}
N 280 870 280 930 {lab=}
N 360 500 360 500 {lab=Vout}
N 280 870 330 870 {lab=}
N 280 780 360 780 {lab=}
N 150 780 220 780 {lab=}
N 150 500 150 780 {lab=}
N 360 780 360 840 {lab=}
N 70 500 70 540 {lab=}
N 150 500 220 500 {lab=}
N 70 600 70 650 {lab=}
N 360 900 360 930 {lab=}
N 360 500 360 580 {lab=}
N 360 640 360 650 {lab=}
N 70 500 150 500 {lab=}
N 280 500 360 500 {lab=}
C {qucs_directive.sym} 70 190 0 0 {name=AC1 symname=.AC qucs.type=.AC}
C {gnd.sym} 360 650 0 0 {name=* qucs.type=GND}
C {vsource.sym} 70 570 1 0 {name=V1 value=1 V qucs.type=Vac}
C {gnd.sym} 70 650 0 0 {name=* qucs.type=GND}
C {gnd.sym} 360 930 0 0 {name=* qucs.type=GND}
C {qucs_blackbox.sym} 360 610 0 0 {name=cap_cmim1 symname=cap_cmim qucs.type=Lib}
C {qucs_blackbox.sym} 250 780 1 0 {name=rhigh4 symname=rhigh qucs.type=Lib}
C {gnd.sym} 280 930 0 0 {name=* qucs.type=GND}
C {qucs_blackbox.sym} 360 870 0 0 {name=cap_rfcmim1 symname=cap_rfcmim qucs.type=Lib}
C {gnd.sym} 250 530 0 0 {name=* qucs.type=GND}
C {gnd.sym} 250 810 0 0 {name=* qucs.type=GND}
C {qucs_blackbox.sym} 250 500 1 0 {name=rhigh3 symname=rhigh qucs.type=Lib}
C {qucs_blackbox.sym} 90 60 0 0 {name=SpiceLib1 symname=res_typ qucs.type=SpiceLib}
C {qucs_blackbox.sym} 280 60 0 0 {name=SpiceLib2 symname=cap_typ qucs.type=SpiceLib}
