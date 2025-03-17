module simple_circuit (
    input wire IN2,
    output wire OUT
);

    INV_X1 inv (.A(IN2),.ZN(OUT));   

endmodule


