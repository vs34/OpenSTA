module top (
  input N1, N2, N3, N6, N7,
  output N22, N23
);

  wire N10, N11, N16, N19;

  nand2x1_sc U1 (
    .A (N1),
    .B (N3),
    .Y (N10)
  );

  nand2x1_sc U2 (
    .A (N3),
    .B (N6),
    .Y (N11)
  );

  nand2x1_sc U3 (
    .A (N2),
    .B (N11),
    .Y (N16)
  );

  nand2x1_sc U4 (
    .A (N11),
    .B (N7),
    .Y (N19)
  );

  nand2x1_sc U5 (
    .A (N10),
    .B (N16),
    .Y (N22)
  );

  nand2x1_sc U6 (
    .A (N16),
    .B (N19),
    .Y (N23)
  );

endmodule

