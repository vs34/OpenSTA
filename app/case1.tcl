# 1) Read in your Liberty
read_liberty lib_7nm.lib

# 2) Read in your netlist
read_verilog logic_net.v

# 3) Link the design
link_design top

# 4) Create a virtual clock (no FFs but needed to build paths)
create_clock -name virtual_clk -period 1.0   ;# 1 ns period

# 5) Set input slews (MIS case 1: A1 slow, A2 fast)
set_input_transition 0.016 [get_ports A1]
set_input_transition 0.021 [get_ports A2]

# 6) Set input delays to define timing startpoints more temporal distance
set_input_delay 1.0 -clock virtual_clk [get_ports A1]
set_input_delay 0.0 -clock virtual_clk [get_ports A2]

# 7) Set load at ZN (fake output environment)
set_load 0.0236 [get_ports ZN]
set_output_delay 0.0 -clock virtual_clk [get_ports ZN]

# 8) Report max delay paths
report_checks -rise_through ZN -path_delay max -format full -digits 5

exit
