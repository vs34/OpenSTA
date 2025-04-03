# Load the Liberty file (Cell Library)
read_liberty typical.lib

# Read the Verilog netlist
read_verilog flipflop.v  ;# Change from complex.v to flipflop.v

# Link the design
link_design simple_netlist  ;# Ensure this matches the module name in flipflop.v

# Create a clock for the CLK signal
create_clock -name CLK -period 1 [get_ports CLK]

# Set input delays on data input ports (assuming single input D)
set_input_delay -clock CLK 1 [get_ports {D}]

# Set output delay on the output port
set_output_delay -clock CLK 1 [get_ports {Q}]

# Perform timing analysis
report_checks

