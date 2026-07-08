v {xschem version=Schematic 24.3.0 file_version=1.2}
G {}
K {schematic.header=<Qucs Schematic 24.3.0> schematic.view=<View=-42,-6,1577,932,0.96905,0,1> schematic.view=<Grid=10,10,1> schematic.view=<DataSet=dc_diode_op.dat> schematic.view=<DataDisplay=dc_diode_op.dpl> schematic.view=<OpenDisplay=0> schematic.view=<Script=diode.m> schematic.view=<RunScript=0> schematic.view=<showFrame=3> schematic.view=<FrameText0=DC simulation of  ESD diodes> schematic.view=<FrameText1=Drawn By:IHP PDK Authors> schematic.view=<FrameText2=Date:2024> schematic.view=<FrameText3=Revision:1>}
V {}
S {}
E {}
N 470 790 470 840 {lab=}
N 120 590 270 590 {lab=}
N 270 590 270 610 {lab=}
N 270 670 270 730 {lab=}
N 120 780 120 840 {lab=}
N 120 590 120 720 {lab=}
N 470 590 470 610 {lab=}
N 270 790 270 840 {lab=}
N 270 590 470 590 {lab=}
N 470 670 470 730 {lab=}
C {qucs_directive.sym} 90 280 0 0 {name=SW1 symname=.SW qucs.type=.SW}
C {gnd.sym} 270 840 0 0 {name=* qucs.type=GND}
C {ngspice_probe.sym} 270 640 3 0 {name=Pr1 qucs.type=IProbe}
C {qucs_blackbox.sym} 130 50 0 0 {name=INCLSCR1 symname=INCLSCR qucs.type=INCLSCR}
C {gnd.sym} 120 840 0 0 {name=* qucs.type=GND}
C {vsource.sym} 120 750 1 0 {name=V2 value=1 V qucs.type=Vdc}
C {gnd.sym} 470 840 0 0 {name=* qucs.type=GND}
C {ngspice_probe.sym} 470 640 3 0 {name=Pr2 qucs.type=IProbe}
C {qucs_blackbox.sym} 270 760 3 0 {name=dantenna1 symname=dantenna qucs.type=Lib}
C {qucs_blackbox.sym} 470 760 3 0 {name=dpantenna1 symname=dpantenna qucs.type=Lib}
