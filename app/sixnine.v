module simple_circuit (
    input wire IN2,
    output wire OUT
);

    wire loop_net;

    AND2_X1 u1 (.A1(IN2), .A2(loop_net), .ZN(loop_net)); // Combinational loop
    assign OUT = loop_net;

endmodule

