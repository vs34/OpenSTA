# 1) Read in the Liberty file
read_liberty lib_7nm.lib

# 2) Read in your netlist
read_verilog benchmark.v  ;# Ensure benchmark.v contains the 'c17' module

# 3) Link the top-level design
link_design top

# 4) Create a virtual clock for timing analysis (needed for start/end points)
create_clock -name virtual_clk -period 1.0 [get_ports N1]  ;# 1 ns period

# 5) Set input transition times (slews)
# MIS Case 1: Slow N1, Fast N2-N7
set_input_transition 0.080 [get_ports N1]
set_input_transition 0.080 [get_ports {N2 N3 N6 N7}]

# 6) Set input delays to define timing startpoints
set_input_delay 0.0 -clock virtual_clk [get_ports {N1 N2 N3 N6 N7}]

# 7) Set output loads and delays (fake output environment)
set_load 0.020 [get_ports {N22 N23}]  ;# 0.1 fF cap
set_output_delay 0.0 -clock virtual_clk [get_ports {N22 N23}]

# 8) Report max delay paths (for both outputs)

# 10) Re-run delay path analysis

report_checks -from [get_pins U5/A] -through [get_pins U5/Y] -path_delay max -format full -digits 5
