// ============================================================================
// MODULE: shift_rows  (AES ShiftRows transformation)
// ----------------------------------------------------------------------------
// Pure wiring: NO gates, NO clock, NO registers. Purely combinational/interconnect.
//
// Byte numbering (FIPS-197 order, column-major):
//   byte index i = row + 4*col   -> state[row][col]
//   in[127:120] = byte 0 (r0,c0), in[119:112] = byte 1 (r1,c0) ... in[7:0] = byte 15 (r3,c3)
//
//   Input matrix            Output matrix (row r rotated left by r)
//   b0  b4  b8  b12         b0   b4   b8   b12
//   b1  b5  b9  b13   -->   b5   b9   b13  b1
//   b2  b6  b10 b14         b10  b14  b2   b6
//   b3  b7  b11 b15         b15  b3   b7   b11
// ============================================================================
module shift_rows (
    input  wire [127:0] state_in,
    output wire [127:0] state_out
);

    // Split the input bus into 16 named bytes
    wire [7:0] b0  = state_in[127:120];
    wire [7:0] b1  = state_in[119:112];
    wire [7:0] b2  = state_in[111:104];
    wire [7:0] b3  = state_in[103:96];
    wire [7:0] b4  = state_in[95:88];
    wire [7:0] b5  = state_in[87:80];
    wire [7:0] b6  = state_in[79:72];
    wire [7:0] b7  = state_in[71:64];
    wire [7:0] b8  = state_in[63:56];
    wire [7:0] b9  = state_in[55:48];
    wire [7:0] b10 = state_in[47:40];
    wire [7:0] b11 = state_in[39:32];
    wire [7:0] b12 = state_in[31:24];
    wire [7:0] b13 = state_in[23:16];
    wire [7:0] b14 = state_in[15:8];
    wire [7:0] b15 = state_in[7:0];

    // Re-wire (just reorder the wires)
    assign state_out = {
        // column 0: row0 (no shift), row1 (<-b5), row2 (<-b10), row3 (<-b15)
        b0,  b5,  b10, b15,
        // column 1
        b4,  b9,  b14, b3,
        // column 2
        b8,  b13, b2,  b7,
        // column 3
        b12, b1,  b6,  b11
    };

endmodule
