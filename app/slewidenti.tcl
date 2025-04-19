# Load the Liberty file
read_liberty typical.lib

# Read the Verilog netlist
read_verilog small.v

# Link the design
link_design simple_circuit

# Create a clock
create_clock -name CLK -period 0.1 

# Slew annotations
set_input_transition -rise 0.12434 [get_ports IN2]
set_input_transition -fall 0.9798 [get_ports IN2]

# Input delays on IN2
set_input_delay -min -rise -clock CLK 0.133 [get_ports {IN2}]
set_input_delay -max -rise -clock CLK 0.237 [get_ports {IN2}]
set_input_delay -min -fall -clock CLK 0.331 [get_ports {IN2}]
set_input_delay -max -fall -clock CLK 0.401 [get_ports {IN2}]

# Output delay
set_output_delay -clock CLK 0.01 [get_ports {OUT}]

# Report min delay
report_checks -path_delay min -digit 9

