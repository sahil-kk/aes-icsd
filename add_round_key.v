module add_round_key (
    input  wire [127:0] state_in,
    input  wire [127:0] round_key,
    output wire [127:0] state_out
);

    // Pure combinational logic: 128 two-input XOR gates in parallel
    assign state_out = state_in ^ round_key;

endmodule
