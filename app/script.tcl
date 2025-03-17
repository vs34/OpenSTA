# Load the Liberty file
read_liberty typical.lib

# Read the Verilog netlist
read_verilog netlist.v

# Link the design
link_design netlist

# Create a clock for the CLK signal
create_clock -name CLK -period 10 [get_ports CLK]

# Set input delays on data input ports
set_input_delay -clock CLK 1 [get_ports {IN1 IN2 IN3 IN4 IN5 IN6 IN7 IN8 IN9}]

# Set output delays on the output port
set_output_delay -clock CLK 1 [get_ports {OUT}]

# Perform timing analysis
report_checks -path_delay min

