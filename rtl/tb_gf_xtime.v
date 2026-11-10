`timescale 1ns / 1ps

module tb_gf_xtime;
    reg  [7:0] in;
    wire [7:0] out;

    integer value;
    integer errors;
    reg [8:0] polynomial;
    reg [7:0] expected;

    gf_xtime dut (
        .in(in),
        .out(out)
    );

    initial begin
        in = 8'h00;
        errors = 0;

        for (value = 0; value < 256; value = value + 1) begin
            in = value[7:0];

            polynomial = {in, 1'b0};
            if (polynomial[8])
                polynomial = polynomial ^ 9'h11B;
            expected = polynomial[7:0];

            #1;
            if (out !== expected) begin
                $display("FAIL: in=%02h expected=%02h actual=%02h",
                         in, expected, out);
                errors = errors + 1;
            end
        end

        if (errors == 0)
            $display("PASS: all 256 gf_xtime inputs matched.");
        else
            $display("FAIL: %0d of 256 inputs mismatched.", errors);

        $finish;
    end
endmodule
