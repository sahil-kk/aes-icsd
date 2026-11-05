module tb_sbox;

reg [7:0] in;
reg mode;
wire [7:0] out;

reg [7:0] temp;
integer i;
integer errors;

sbox uut (
    .in(in),
    .mode(mode),
    .out(out)
);

initial begin
    errors = 0;

    // Test all 256 forward and inverse combinations
    mode = 0;

    for (i = 0; i < 256; i = i + 1) begin
        in = i;
        #10;

        temp = out;

        mode = 1;
        in = temp;
        #10;

        if (out !== i) begin
            errors = errors + 1;
            $display("ERROR: Input=%h, Output=%h", i, out);
        end

        mode = 0;
    end

    if (errors == 0)
        $display("ALL 256 ROUND-TRIP TESTS PASSED");
    else
        $display("TESTS FAILED: %0d errors", errors);

end

endmodule