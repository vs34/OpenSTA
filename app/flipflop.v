module simple_netlist (
    input CLK,  // Clock input
    input D,    // Data input
    output Q    // Output
);

    // Single D Flip-Flop
    DFF_X1 FF1 (.D(D), .CK(CLK), .Q(Q));

endmodule

