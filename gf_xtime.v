// Multiply one byte by 2 in AES GF(2^8).
// AES uses the reduction polynomial x^8 + x^4 + x^3 + x + 1 (0x11B).
module gf_xtime (
    input  wire [7:0] in,
    output wire [7:0] out
);
    // Shift left, then reduce if the original most significant bit was set.
    assign out = {in[6:0], 1'b0} ^ (in[7] ? 8'h1B : 8'h00);
endmodule
