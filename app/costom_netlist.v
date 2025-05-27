module logic_net (
    input  wire IN1,
    input  wire IN2,
    input  wire Clk,
    output wire OUT
);

    // Intermediate nets
    wire op_f1, op_nand, op_buf;

    // D‐flip‐flops from your SC library
    // Note: I’m assuming your SC library names them DFF_SC (or DFF_X1_SC)
    dff_sc FF1 (
        .CLK  (Clk),
        .D   (IN1),
        .Q   (op_f1)
    );

    dff_sc FF2 (
        .CLK  (Clk),
        .D   (op_nand),
        .Q   (OUT)
    );

    // Buffered version of IN2  this buffer from the liberty is broken
    // buffer_sc BUF1 (
        // .A   (IN2),
        // .Z   (op_buf)
    // );

    // 2-input NAND
    nand2x1_sc NAND1 (
        .A  (op_f1),
        .B  (IN2),
        .Y  (op_nand)
    );

endmodule

