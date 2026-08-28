v {xschem version=Schematic 24.3.0 file_version=1.2}
G {}
K {schematic.header=<Qucs Schematic 24.3.0> schematic.view=<View=-42,-6,1577,932,0.96905,0,1> schematic.view=<Grid=10,10,1> schematic.view=<DataSet=dc_hv_nmos.dat> schematic.view=<DataDisplay=dc_hv_nmos.dpl> schematic.view=<OpenDisplay=0> schematic.view=<Script=dc_hv_nmos.m> schematic.view=<RunScript=0> schematic.view=<showFrame=3> schematic.view=<FrameText0=DC simulation of a High Voltage  N type MOS> schematic.view=<FrameText1=Drawn By:IHP PDK Authors> schematic.view=<FrameText2=Date:2024> schematic.view=<FrameText3=Revision:1>}
V {}
S {}
E {}
N 330 750 330 840 {lab=}
N 270 780 270 840 {lab=}
N 290 750 330 750 {lab=}
N 270 670 270 720 {lab=}
N 160 750 240 750 {lab=}
N 270 590 450 590 {lab=}
N 270 590 270 610 {lab=}
N 450 590 450 720 {lab=}
N 450 780 450 840 {lab=}
N 160 830 160 840 {lab=}
N 160 750 160 770 {lab=}
C {qucs_directive.sym} 90 280 0 0 {name=SW1 symname=.SW qucs.type=.SW}
C {gnd.sym} 270 840 0 0 {name=* qucs.type=GND}
C {gnd.sym} 330 840 0 0 {name=* qucs.type=GND}
C {ngspice_probe.sym} 270 640 3 0 {name=Pr1 qucs.type=IProbe}
C {vsource.sym} 450 750 1 0 {name=V2 value=1 V qucs.type=Vdc}
C {gnd.sym} 450 840 0 0 {name=* qucs.type=GND}
C {vsource.sym} 160 800 1 0 {name=V1 value=1 V qucs.type=Vdc}
C {gnd.sym} 160 840 0 0 {name=* qucs.type=GND}
C {qucs_blackbox.sym} 130 50 0 0 {name=INCLSCR1 symname=INCLSCR qucs.type=INCLSCR}
C {qucs_directive.sym} 90 180 0 0 {name=DC1 symname=.DC qucs.type=.DC}
C {qucs_directive.sym} 260 280 0 0 {name=SW2 symname=.SW qucs.type=.SW}
C {qucs_blackbox.sym} 270 750 0 0 {name=sg13_hv_nmos1 symname=sg13_hv_nmos qucs.type=Lib}
