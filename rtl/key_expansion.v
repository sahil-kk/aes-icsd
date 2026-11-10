// ============================================================================
// MODULE: key_expansion  (AES-128 key schedule)
// Verilog translation of key_expansion.c
//
// One round key is produced per clock cycle by a single shared datapath
// feeding back through a 128-bit register.
//
// Byte order: key[127:120] = key byte 0 (matches the FIPS-197 hex string).
//   w0 = key[127:96]  (column 0, bytes 0..3)
//   w1 = key[95:64]   (column 1)
//   w2 = key[63:32]   (column 2)
//   w3 = key[31:0]    (column 3, bytes 12..15)
//   First byte of each word = its top 8 bits.
//
// all_keys layout: round r occupies all_keys[1407-128*r -: 128]
//   round 0  -> all_keys[1407:1280]
//   round 10 -> all_keys[127:0]
//
// Timing: assert 'start' for one cycle. 'ready' goes low immediately and
// returns high after round 10 is stored (10 cycles after the start cycle).
// 'ready' stays high until the next 'start'.
//
// Requires: sbox.v  ->  module sbox(input [7:0] in, output [7:0] out)
// ============================================================================
module key_expansion (
    input              clk,
    input              rst_n,
    input              start,
    input      [127:0] key,
    output reg [1407:0] all_keys,
    output reg         ready
);

    // ------------------------------------------------------------------
    // State
    // ------------------------------------------------------------------
    reg [127:0] rk;      // previous round key (feeds the datapath)
    reg [3:0]   round;   // round number of the key being computed (1..10)
    reg         busy;

    // ------------------------------------------------------------------
    // Rcon ROM (index = round number 1..10)
    // ------------------------------------------------------------------
    reg [7:0] rcon;
    always @(*) begin
        case (round)
            4'd1:    rcon = 8'h01;
            4'd2:    rcon = 8'h02;
            4'd3:    rcon = 8'h04;
            4'd4:    rcon = 8'h08;
            4'd5:    rcon = 8'h10;
            4'd6:    rcon = 8'h20;
            4'd7:    rcon = 8'h40;
            4'd8:    rcon = 8'h80;
            4'd9:    rcon = 8'h1B;
            4'd10:   rcon = 8'h36;
            default: rcon = 8'h00;
        endcase
    end

    // ------------------------------------------------------------------
    // Combinational next-round-key datapath
    // ------------------------------------------------------------------
    wire [31:0] w0 = rk[127:96];
    wire [31:0] w1 = rk[95:64];
    wire [31:0] w2 = rk[63:32];
    wire [31:0] w3 = rk[31:0];

    // rot_word: [b0 b1 b2 b3] -> [b1 b2 b3 b0]  (pure wiring)
    wire [31:0] rotated = {w3[23:0], w3[31:24]};

    // sub_word: 4 parallel S-boxes
    wire [31:0] substituted;
    sbox sb0 (.in(rotated[31:24]), .out(substituted[31:24]));
    sbox sb1 (.in(rotated[23:16]), .out(substituted[23:16]));
    sbox sb2 (.in(rotated[15:8]),  .out(substituted[15:8]));
    sbox sb3 (.in(rotated[7:0]),   .out(substituted[7:0]));

    // Rcon XOR into the first byte only
    wire [31:0] transformed = {substituted[31:24] ^ rcon, substituted[23:0]};

    // Column chaining
    wire [31:0] n0 = w0 ^ transformed;
    wire [31:0] n1 = w1 ^ n0;
    wire [31:0] n2 = w2 ^ n1;
    wire [31:0] n3 = w3 ^ n2;

    wire [127:0] next_key = {n0, n1, n2, n3};

    // ------------------------------------------------------------------
    // Sequencer
    // ------------------------------------------------------------------
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            rk       <= 128'b0;
            round    <= 4'd0;
            busy     <= 1'b0;
            ready    <= 1'b0;
            all_keys <= {1408{1'b0}};
        end else if (start && !busy) begin
            // Round 0 key is the master key itself
            rk                    <= key;
            all_keys[1407 -: 128] <= key;
            round                 <= 4'd1;
            busy                  <= 1'b1;
            ready                 <= 1'b0;
        end else if (busy) begin
            rk                                 <= next_key;
            all_keys[1407 - 128*round -: 128]  <= next_key;
            if (round == 4'd10) begin
                busy  <= 1'b0;
                ready <= 1'b1;
            end else begin
                round <= round + 4'd1;
            end
        end
    end

endmodule
