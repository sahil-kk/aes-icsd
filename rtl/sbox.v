
module sbox (
    input wire [7:0] in,
    input wire mode,
    output reg [7:0] out
);

reg [7:0] sbox_table [0:255];
reg [7:0] inv_sbox_table [0:255];

initial begin

    // Forward S-Box
    $readmemh("sbox.hex", sbox_table);

    // Inverse S-Box
    $readmemh("inv_sbox.hex", inv_sbox_table);

end

always @(*) begin
    if (mode == 1'b0)
        out = sbox_table[in];
    else
        out = inv_sbox_table[in];
end

endmodule

