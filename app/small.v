module simple_circuit (
    input wire IN2,
    output wire OUT
);

    wire in,out;
    assign in = IN2;
    INV_X1 inv (.A(in),.ZN(out));   
    assign OUT = out;

endmodule


