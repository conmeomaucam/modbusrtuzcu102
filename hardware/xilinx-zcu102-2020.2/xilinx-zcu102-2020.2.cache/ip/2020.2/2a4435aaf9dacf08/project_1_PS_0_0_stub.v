// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2020.2 (lin64) Build 3064766 Wed Nov 18 09:12:47 MST 2020
// Date        : Wed Nov 18 22:57:46 2020
// Host        : xcosswbld06 running 64-bit Red Hat Enterprise Linux Workstation release 7.4 (Maipo)
// Command     : write_verilog -force -mode synth_stub -rename_top decalper_eb_ot_sdeen_pot_pi_dehcac_xnilix -prefix
//               decalper_eb_ot_sdeen_pot_pi_dehcac_xnilix_ project_1_PS_0_0_stub.v
// Design      : project_1_PS_0_0
// Purpose     : Stub declaration of top-level module interface
// Device      : xczu9eg-ffvb1156-2-e
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* X_CORE_INFO = "zynq_ultra_ps_e_v3_3_3_zynq_ultra_ps_e,Vivado 2020.2" *)
module decalper_eb_ot_sdeen_pot_pi_dehcac_xnilix(pl_ps_irq0, pl_resetn0, pl_resetn1, pl_resetn2, 
  pl_resetn3, pl_clk0, pl_clk1, pl_clk2, pl_clk3)
/* synthesis syn_black_box black_box_pad_pin="pl_ps_irq0[0:0],pl_resetn0,pl_resetn1,pl_resetn2,pl_resetn3,pl_clk0,pl_clk1,pl_clk2,pl_clk3" */;
  input [0:0]pl_ps_irq0;
  output pl_resetn0;
  output pl_resetn1;
  output pl_resetn2;
  output pl_resetn3;
  output pl_clk0;
  output pl_clk1;
  output pl_clk2;
  output pl_clk3;
endmodule
