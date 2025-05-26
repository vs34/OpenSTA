module logic_net (
    input wire IN1,
    input wire IN2,
    input wire Clk,
    output wire OUT
);

    // Intermediate wires, like verses linking stanzas
    wire op_f1, op_nand, op_buf;

    // Flip-Flop 1 — echoing IN1 through the clocked breeze
    DFF_X1 FF1 (
        .CK(Clk),
        .D(IN1),
        .Q(op_f1)
    );

    // Flip-Flop 2 — dancing with IN2, edge-bound
    DFF_X1 FF2 (
        .CK(Clk),
        .D(op_nand),
        .Q(OUT)
    );

    // Buffer — calming the signal, flowing smooth
    BUF_X1 BUF1 (
        .A(IN2),
        .Z(op_buf)
    );

    // NAND Gate — where op_f1 meets op_buf in dark logic embrace
    NAND2_X1 NAND1 (
        .A1(op_f1),
        .A2(op_buf),
        .ZN(op_nand)
    );

endmodule

