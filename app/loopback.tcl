# Load the Liberty file
read_liberty typical.lib

# Read the Verilog netlist
read_verilog loopback.v

# Link the design
link_design loopback

# Create a clock for the CLK signal
create_clock -name CLK -period 0.1 

# Set input delays on data input ports
set_input_delay -min -rise -clock CLK 0.133 [get_ports {IN}]
set_input_delay -max -rise -clock CLK 0.237 [get_ports {IN2}]
set_input_delay -min -fall -clock CLK 0.331 [get_ports {IN2}]
set_input_delay -max -fall -clock CLK 0.401 [get_ports {IN2}]

# Set output delays on the output port
set_output_delay -clock CLK 0.01 [get_ports {OUT}]

# Report minimum (early) path delays
report_checks -path_delay min -digit 9
# report_checks -path_delay max -digit 9
# report_checks -fields slew -digit 9

# report_checks -from [get_pins FF7/Q] -to [get_pins FF8/D] -path_delay min


# Report maximum (late) path delays
# report_checks -path_delay max
