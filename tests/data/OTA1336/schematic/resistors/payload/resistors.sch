v {xschem version=Schematic 24.4.1 file_version=1.2}
G {}
K {schematic.header=<Qucs Schematic 24.4.1> schematic.view=<View=-255,-70,2012,1279,0.723747,0,0> schematic.view=<Grid=10,10,1> schematic.view=<DataSet=resistors.dat> schematic.view=<DataDisplay=resistors.dpl> schematic.view=<OpenDisplay=0> schematic.view=<Script=resistors.m> schematic.view=<RunScript=0> schematic.view=<showFrame=3> schematic.view=<FrameText0=DC simulation resistors> schematic.view=<FrameText1=Drawn By:IHP PDK Authors> schematic.view=<FrameText2=Date:2024> schematic.view=<FrameText3=Revision:1>}
V {}
S {}
E {}
N 510 550 570 550 {lab=}
N 510 410 510 550 {lab=}
N 330 410 330 550 {lab=}
N 330 410 380 410 {lab=}
N 180 410 210 410 {lab=}
N 1200 650 1200 670 {lab=}
N 1120 650 1200 650 {lab=ptap}
N 330 550 380 550 {lab=}
N 1200 730 1200 750 {lab=}
N 970 650 970 670 {lab=}
N 1120 650 1120 690 {lab=}
N 240 580 240 620 {lab=}
N 600 360 600 380 {lab=}
N 180 550 210 550 {lab=}
N 410 360 410 380 {lab=}
N 240 360 240 380 {lab=}
N 510 550 510 700 {lab=}
N 200 360 240 360 {lab=}
N 100 360 100 440 {lab=}
N 510 410 570 410 {lab=}
N 100 500 100 620 {lab=}
N 100 360 140 360 {lab=}
N 180 410 180 550 {lab=}
N 240 360 410 360 {lab=}
N 600 580 600 620 {lab=}
N 410 580 410 620 {lab=}
N 410 360 600 360 {lab=}
N 410 440 410 520 {lab=div2}
N 600 440 600 520 {lab=div3}
N 180 550 180 700 {lab=}
N 870 650 970 650 {lab=ntap}
N 970 730 970 750 {lab=}
N 240 440 240 520 {lab=div1}
N 330 550 330 700 {lab=}
N 870 650 870 690 {lab=}
C {qucs_blackbox.sym} 130 50 0 0 {name=INCLSCR1 symname=INCLSCR qucs.type=INCLSCR rotate=0 mirror=0}
C {qucs_directive.sym} 320 70 0 0 {name=SW1 symname=.SW qucs.type=.SW rotate=0 mirror=0}
C {vsource.sym} 100 470 0 0 {name=V2 value=1 V qucs.type=Vdc rotate=1 mirror=0}
C {gnd.sym} 240 620 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {gnd.sym} 100 620 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {IProbe.sym} 170 360 0 0 {name=Pr1 qucs.type=IProbe rotate=0 mirror=0}
C {gnd.sym} 410 620 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {gnd.sym} 600 620 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {rsil.sym} 240 550 0 0 {name=rsil2 qucs.type=Lib qucs.model=rsil rotate=0 mirror=0}
C {rppd.sym} 410 410 0 0 {name=rppd1 qucs.type=Lib qucs.model=rppd rotate=0 mirror=0}
C {rhigh.sym} 600 410 0 0 {name=rhigh1 qucs.type=Lib qucs.model=rhigh rotate=0 mirror=0}
C {rppd.sym} 410 550 0 0 {name=rppd2 qucs.type=Lib qucs.model=rppd rotate=0 mirror=0}
C {gnd.sym} 970 750 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {gnd.sym} 870 750 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {gnd.sym} 1200 750 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {gnd.sym} 1120 750 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {isource.sym} 870 720 0 0 {name=I1 value=1 mA qucs.type=Idc rotate=1 mirror=0}
C {isource.sym} 1120 720 0 0 {name=I2 value=1 mA qucs.type=Idc rotate=1 mirror=0}
C {ptap1.sym} 970 700 0 0 {name=ptap1 qucs.type=Lib qucs.model=ptap1 rotate=0 mirror=0}
C {ntap1.sym} 1200 700 0 0 {name=ntap1 qucs.type=Lib qucs.model=ntap1 rotate=0 mirror=0}
C {ptap1.sym} 180 730 0 0 {name=ptap4 qucs.type=Lib qucs.model=ptap1 rotate=0 mirror=0}
C {ptap1.sym} 330 730 0 0 {name=ptap3 qucs.type=Lib qucs.model=ptap1 rotate=0 mirror=0}
C {ptap1.sym} 510 730 0 0 {name=ptap2 qucs.type=Lib qucs.model=ptap1 rotate=0 mirror=0}
C {gnd.sym} 180 760 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {gnd.sym} 510 760 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {gnd.sym} 330 760 0 0 {name=* qucs.type=GND rotate=0 mirror=0}
C {rhigh.sym} 600 550 0 0 {name=rhigh2 qucs.type=Lib qucs.model=rhigh rotate=0 mirror=0}
C {rsil.sym} 240 410 0 0 {name=rsil1 qucs.type=Lib qucs.model=rsil rotate=0 mirror=0}
