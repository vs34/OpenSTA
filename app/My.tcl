











read_liberty    costom.lib
read_verilog    costom_netlist.v
link_design     logic_net

# 2) Clock definition (tight to magnify MIS effects)
create_clock -name Clk -period 0.1 [get_ports Clk]
set_clock_uncertainty 0.02 [get_clocks Clk]

# 3) Force aggressive simultaneous transitions on IN1 & IN2 (5 ps)
set_input_transition 0.005 [get_ports IN1]
set_input_transition 0.005 [get_ports IN2]

# 4) Model a small output load (20 fF assuming typical.lib uses fF)
set_load 0.02 [get_ports OUT]

report_checks -rise_through  NAND1/A -digits 5 -format full -fields net -path_delay max
report_checks -rise_through  NAND1/B -digits 5 -format full -fields net -path_delay max

report_checks -rise_through  NAND1/A -digits 5 -format full -fields net -path_delay max


report_checks
