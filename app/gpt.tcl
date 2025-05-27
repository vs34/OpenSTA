# Don't define a clock at all
read_liberty lib_7nm.lib
read_verilog logic_net.v
link_design top

create_clock -name Clk -period 0.1 [get_ports Clk]
set_clock_uncertainty 0.02 [get_clocks Clk]

set_input_transition 0.2 [get_ports A1]
set_input_transition 0.05 [get_ports A2]
set_load 0.1 [get_ports ZN]

# Now report delay through the combinational path
report_checks -from [get_ports A1] -to [get_ports ZN] -path_delay max -format full -digits 5
report_checks -from [get_ports A2] -to [get_ports ZN] -path_delay max -format full -digits 5

