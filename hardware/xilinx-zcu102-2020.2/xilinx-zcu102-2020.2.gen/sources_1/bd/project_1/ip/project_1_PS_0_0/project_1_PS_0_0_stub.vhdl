-- Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2020.2 (lin64) Build 3064766 Wed Nov 18 09:12:47 MST 2020
-- Date        : Wed Nov 18 22:57:47 2020
-- Host        : xcosswbld06 running 64-bit Red Hat Enterprise Linux Workstation release 7.4 (Maipo)
-- Command     : write_vhdl -force -mode synth_stub
--               /tmp/tmp.78I84wEmX4/temp/hwflow/hwflow_project_1/project_1.gen/sources_1/bd/project_1/ip/project_1_PS_0_0/project_1_PS_0_0_stub.vhdl
-- Design      : project_1_PS_0_0
-- Purpose     : Stub declaration of top-level module interface
-- Device      : xczu9eg-ffvb1156-2-e
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity project_1_PS_0_0 is
  Port ( 
    pl_ps_irq0 : in STD_LOGIC_VECTOR ( 0 to 0 );
    pl_resetn0 : out STD_LOGIC;
    pl_resetn1 : out STD_LOGIC;
    pl_resetn2 : out STD_LOGIC;
    pl_resetn3 : out STD_LOGIC;
    pl_clk0 : out STD_LOGIC;
    pl_clk1 : out STD_LOGIC;
    pl_clk2 : out STD_LOGIC;
    pl_clk3 : out STD_LOGIC
  );

end project_1_PS_0_0;

architecture stub of project_1_PS_0_0 is
attribute syn_black_box : boolean;
attribute black_box_pad_pin : string;
attribute syn_black_box of stub : architecture is true;
attribute black_box_pad_pin of stub : architecture is "pl_ps_irq0[0:0],pl_resetn0,pl_resetn1,pl_resetn2,pl_resetn3,pl_clk0,pl_clk1,pl_clk2,pl_clk3";
attribute X_CORE_INFO : string;
attribute X_CORE_INFO of stub : architecture is "zynq_ultra_ps_e_v3_3_3_zynq_ultra_ps_e,Vivado 2020.2";
begin
end;
