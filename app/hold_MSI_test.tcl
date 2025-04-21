# Load the Liberty file
read_liberty typical.lib

# Read the Verilog netlist
read_verilog hold_MSI_test.v

# Link the design
link_design logic_net

# Create a clock for the Clk signal
create_clock -name Clk -period 1 [get_ports Clk]

# Set input delays on data input ports
set_input_delay -clock Clk 1 [get_ports {IN1 IN2}]

# Set output delays on the output port
set_output_delay -clock Clk 1 [get_ports {OUT}]

# Perform timing analysis
report_checks

