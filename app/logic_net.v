//------------------------------------------------------------------------------
// Simple top‐level that instantiates a single 2‐input NAND from your SC library.
//------------------------------------------------------------------------------
module top (
  input  wire A1,
  input  wire A2,
  output wire ZN
);

  // Instantiate the standard‐cell NAND2 from your SC library.
  // Note: the port names in the instance must match your cell definition.
  nand2x1_sc NAND1 (
    .A   (A1),
    .B   (A2),
    .Y  (ZN)
  );

endmodule

