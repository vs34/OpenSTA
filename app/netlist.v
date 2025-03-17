module netlist (
    input IN1, IN2, IN3, IN4, IN5, IN6, IN7, IN8,
    output OUT
);

    wire op1, op2, op3, op4, op5, op6, op7, op8;

    // First stage
    AND2_X1 i1 (
        .A1(IN2),
        .A2(IN2),
        .ZN(op1)
    );

    // Second stage
    AND2_X1 i2 (
        .A1(op1),
        .A2(op1),
        .ZN(op2)
    );

    // Third stage
    AND2_X1 i3 (
        .A1(op2),
        .A2(IN1),
        .ZN(op3)
    );

    // Fourth stage
    AND2_X1 i4 (
        .A1(op3),
        .A2(IN3),
        .ZN(op4)
    );

    // Fifth stage
    AND2_X1 i5 (
        .A1(op4),
        .A2(IN4),
        .ZN(op5)
    );

    // Sixth stage
    AND2_X1 i6 (
        .A1(op5),
        .A2(IN5),
        .ZN(op6)
    );

    // Seventh stage
    AND2_X1 i7 (
        .A1(op6),
        .A2(IN6),
        .ZN(op7)
    );

    // Eighth stage
    AND2_X1 i8 (
        .A1(op7),
        .A2(IN7),
        .ZN(op8)
    );

    // Final stage
    AND2_X1 i9 (
        .A1(op8),
        .A2(IN8),
        .ZN(OUT)
    );

endmodule

