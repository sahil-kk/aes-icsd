`timescale 1ns/1ps

module tb_sbox;

reg [7:0] in;
reg mode;

wire [7:0] out;
wire [7:0] default_out;

reg [7:0] temp;
integer i;
integer errors;

sbox uut (
    .in(in),
    .mode(mode),
    .out(out)
);

// Intentionally leave mode unconnected
sbox uut_default (
    .in(in),
    .out(default_out)
);

initial begin

    errors = 0;

    // Test unconnected mode
    in = 8'h00;
    mode = 1'b0;

    #10;

    $display("Default instance mode = %b", uut_default.mode);
    $display("Default output = %h", default_out);

    if (default_out !== 8'h63) begin
        errors = errors + 1;
        $display("ERROR: Default mode failed. Expected 63, Got %h",
                 default_out);
    end
    else begin
        $display("DEFAULT MODE TEST PASSED");
    end

    // Direct forward S-Box reference test
    mode = 1'b0;
    in = 8'h53;
    #10;

    if (out !== 8'hED) begin
        errors = errors + 1;
        $display("FORWARD ERROR: Input=53, Expected ED, Got %h", out);
    end
    else begin
        $display("DIRECT FORWARD TEST PASSED");
    end

    // Direct inverse S-Box reference test
    mode = 1'b1;
    in = 8'hED;
    #10;

    if (out !== 8'h53) begin
        errors = errors + 1;
        $display("INVERSE ERROR: Input=ED, Expected 53, Got %h", out);
    end
    else begin
        $display("DIRECT INVERSE TEST PASSED");
    end

    // Test all 256 forward and inverse combinations
    for (i = 0; i < 256; i = i + 1) begin

        mode = 1'b0;
        in = i;
        #10;

        temp = out;

        mode = 1'b1;
        in = temp;
        #10;

        if (out !== i[7:0]) begin
            errors = errors + 1;
            $display("ROUND-TRIP ERROR: Input=%h, Forward=%h, Inverse=%h",
                     i[7:0], temp, out);
        end

    end

    if (errors == 0)
        $display("ALL TESTS PASSED");
    else
        $display("TESTS FAILED: %0d errors", errors);

    $finish;

end

endmodule