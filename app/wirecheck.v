module simple_circuit (
    input wire IN2,
    output wire OUT
);

    wire mid;
    INV_X1 inv1 (.A(IN2),.ZN(mid));   
    INV_X1 inv2 (.A(mid),.ZN(OUT));   

endmodule


