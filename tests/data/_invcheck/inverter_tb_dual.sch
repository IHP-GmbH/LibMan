<Qucs Schematic 24.4.1>
<Properties>
  <View=0,-450,900,100,0.7,0,0>
  <Grid=10,10,1>
  <DataSet=inverter_tb.dat>
  <DataDisplay=inverter_tb.dpl>
  <OpenDisplay=1>
  <showFrame=0>
</Properties>
<Components>
  <INCLSCR INCLSCR1 1 40 -180 -26 16 0 0 ".LIB cornerMOSlv.lib mos_tt\n" 1 "" 0 "" 0>
  <.TR TR1 1 40 -100 0 61 0 0 "lin" 1 "0" 1 "2u" 1 "41" 0 "Trapezoidal" 0 "2" 0 "1 ns" 0 "1e-16" 0 "150" 0 "0.001" 0 "1 pA" 0 "1 uV" 0 "26.85" 0 "1e-3" 0 "1e-6" 0 "1" 0 "CroutLU" 0 "no" 0 "yes" 0 "0" 0>
  <Vdc Vdd 1 70 -110 18 -26 0 1 "1.2 V" 1>
  <GND * 1 110 -50 0 0 0 0>
  <Vpulse Vin 1 150 -110 18 -26 0 1 "0" 1 "1.2" 1 "0.5u" 1 "2u" 1 "10n" 1 "10n" 1>
  <Port p2 1 220 -300 18 -26 0 0 "Vin" 1 "analog" 0 "v" 0 "" 0>
  <Port p4 1 320 -410 18 -26 0 0 "Vdd" 1 "analog" 0 "v" 0 "" 0>
  <Port p5 1 540 -300 18 -26 0 1 "Vout" 1 "analog" 0 "v" 0 "" 0>
  <GND * 1 320 -190 0 0 0 0>
  <Lib x1 1 390 -300 50 -20 0 0 "module_0_foundations" 0 "inverter" 0>
</Components>
<Wires>
  <70 -80 70 -50 "" 0 0 0 "">
  <70 -50 110 -50 "" 0 0 0 "">
  <150 -80 150 -50 "" 0 0 0 "">
  <110 -50 150 -50 "" 0 0 0 "">
  <150 -140 150 -300 "Vin" 165 -220 0 "">
  <150 -300 220 -300 "" 0 0 0 "">
  <220 -300 240 -300 "" 0 0 0 "">
  <70 -140 70 -380 "Vdd" 85 -260 0 "">
  <70 -380 320 -380 "" 0 0 0 "">
  <320 -410 320 -380 "" 0 0 0 "">
  <320 -210 320 -190 "" 0 0 0 "">
  <520 -300 540 -300 "Vout" 530 -285 0 "">
</Wires>
<Diagrams>
  <Rect 600 -180 280 200 3 #c0c0c0 1 00 1 0 0.5 1e-06 1 -0.1 0.1 1.3 1 -1 0.2 1 315 0 225 1 0 0 "" "" "">
	<"ngspice/v(vout)" #0000ff 0 3 0 0 0>
	<"ngspice/v(vin)" #ff0000 0 3 0 0 0>
  </Rect>
</Diagrams>
