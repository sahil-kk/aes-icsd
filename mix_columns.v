
module mix_columns (
    input  wire [127:0] state_in,
    output wire [127:0] state_out
);

// GF(2^8) multiplication by 2
function [7:0] gf_xtime;
    input [7:0] b;
    begin
        gf_xtime = (b[7]) ? ((b << 1) ^ 8'h1b) : (b << 1);
    end
endfunction

// Extract individual columns
wire [31:0] col0 = state_in[127:96];
wire [31:0] col1 = state_in[95:64];
wire [31:0] col2 = state_in[63:32];
wire [31:0] col3 = state_in[31:0];

// MixColumns function for one 32-bit column
function [31:0] mix_single_column;
    input [31:0] c;

    reg [7:0] a0, a1, a2, a3;
    reg [7:0] r0, r1, r2, r3;

    begin
        a0 = c[31:24];
        a1 = c[23:16];
        a2 = c[15:8];
        a3 = c[7:0];

        r0 = gf_xtime(a0) ^ (gf_xtime(a1) ^ a1) ^ a2 ^ a3;
        r1 = a0 ^ gf_xtime(a1) ^ (gf_xtime(a2) ^ a2) ^ a3;
        r2 = a0 ^ a1 ^ gf_xtime(a2) ^ (gf_xtime(a3) ^ a3);
        r3 = (gf_xtime(a0) ^ a0) ^ a1 ^ a2 ^ gf_xtime(a3);

        mix_single_column = {r0, r1, r2, r3};
    end
endfunction

// Apply MixColumns independently to all four columns
assign state_out = {
    mix_single_column(col0),
    mix_single_column(col1),
    mix_single_column(col2),
    mix_single_column(col3)
};

endmodule
