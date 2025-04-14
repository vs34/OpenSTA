
module loopback (
        input wire IN,      // You can try toggling this externally
            output wire OUT     // But it won't make sense in the real world 😅
        );

            wire loop;

                AND2_X1 u1 (
                            .A1(IN),
                                    .A2(loop),
                                            .ZN(loop)    // Loopback: Output ZN drives input A2
                                                );

                                                    assign OUT = loop;

                                                    endmodule

