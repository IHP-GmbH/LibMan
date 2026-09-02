<Qucs Schematic 26.1.1>
<Properties>
  <View=-21,-69,901,617,0.67679,0,0>
  <Grid=10,10,1>
  <DataSet=inverter_tb_qucs_sim.dat>
  <DataDisplay=inverter_tb_qucs_sim.dpl>
  <OpenDisplay=1>
  <Script=inverter_tb.m>
  <RunScript=0>
  <showFrame=0>
  <FrameText0=Title>
  <FrameText1=Drawn By:>
  <FrameText2=Date:>
  <FrameText3=Revision:>
</Properties>
<Symbol>
</Symbol>
<Components>
  <INCLSCR INCLSCR1 1 80 40 -60 16 0 0 ".LIB cornerMOSlv.lib mos_tt\n" 1 "" 0 "" 0>
  <.TR TR1 1 80 140 0 54 0 0 "lin" 1 "0" 1 "2u" 1 "41" 0 "Trapezoidal" 0 "2" 0 "1 ns" 0 "1e-16" 0 "150" 0 "0.001" 0 "1 pA" 0 "1 uV" 0 "26.85" 0 "1e-3" 0 "1e-6" 0 "1" 0 "CroutLU" 0 "no" 0 "yes" 0 "0" 0>
  <Vdc Vdd 1 520 200 18 -26 0 1 "1.2 V" 1>
  <GND * 1 520 260 0 0 0 0>
  <Vpulse Vin 1 120 350 18 -26 0 1 "0" 1 "1.2" 1 "0.5u" 1 "10n" 1 "10n" 1 "1u" 1>
  <GND * 1 120 410 0 0 0 0>
  <GND * 1 330 450 0 0 0 0>
  <Lib Xp 1 330 250 50 -20 1 0 "IHP_PDK_nonlinear_components" 0 "sg13_lv_pmos" 0 "0.45u" 1 "2.0u" 1 "1" 1 "1" 1 "1" 1 "0" 0 "0" 0 "0" 0 "0" 0 "0" 0 "0" 0 "0.34e-6" 0 "0.38e-6" 0 "0.15e-6" 0 "0" 0 "1" 0>
  <Lib Xn 1 330 350 50 -20 0 0 "IHP_PDK_nonlinear_components" 0 "sg13_lv_nmos" 0 "0.45u" 1 "1.0u" 1 "1" 1 "1" 1 "1" 1 "0" 0 "0" 0 "0" 0 "0" 0 "0" 0 "0" 0 "0.34e-6" 0 "0.38e-6" 0 "0.15e-6" 0 "0" 0 "1" 0>
</Components>
<Wires>
  <520 230 520 260 "" 0 0 0 "">
  <120 380 120 410 "" 0 0 0 "">
  <520 140 520 170 "" 0 0 0 "">
  <330 140 520 140 "" 0 0 0 "">
  <330 140 330 220 "" 0 0 0 "">
  <120 320 120 350 "" 0 0 0 "">
  <120 320 300 320 "" 0 0 0 "">
  <300 250 300 320 "" 0 0 0 "">
  <300 250 300 350 "" 0 0 0 "">
  <300 250 310 250 "" 0 0 0 "">
  <300 350 310 350 "" 0 0 0 "">
  <330 280 330 320 "" 0 0 0 "">
  <350 300 420 300 "" 0 0 0 "">
  <330 380 330 450 "" 0 0 0 "">
  <350 250 350 300 "" 0 0 0 "">
  <350 300 350 350 "" 0 0 0 "">
</Wires>
<Diagrams>
  <Rect 480 520 380 280 3 #c0c0c0 1 00 1 0 0.5 1e-06 1 -0.1 0.1 1.3 1 -1 0.2 1 315 0 225 1 0 0 "" "" "">
	<"ngspice/v(vout)" #0000ff 0 3 0 0 0 0>
	<"ngspice/v(vin)" #ff0000 0 3 0 0 0 0>
  </Rect>
</Diagrams>
<Paintings>
</Paintings>
