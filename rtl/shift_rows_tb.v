`timescale 1ns/1ps
// Self-checking testbench for shift_rows
module shift_rows_tb;

    reg  [127:0] state_in;
    wire [127:0] state_out;
    reg  [127:0] expected;

    integer errors;
    integer i, r, c, k;

    shift_rows dut (.state_in(state_in), .state_out(state_out));

    // Reference model: same as the C code. out[r][c] = in[r][(c+r)%4]
    function [127:0] ref_model;
        input [127:0] din;
        integer rr, cc;
        reg [7:0] inb  [0:15];
        reg [7:0] outb [0:15];
        reg [127:0] result;
        begin
            for (rr = 0; rr < 16; rr = rr + 1)
                inb[rr] = din[127 - 8*rr -: 8];
            for (cc = 0; cc < 4; cc = cc + 1)
                for (rr = 0; rr < 4; rr = rr + 1)
                    outb[rr + 4*cc] = inb[rr + 4*((cc + rr) % 4)];
            for (rr = 0; rr < 16; rr = rr + 1)
                result[127 - 8*rr -: 8] = outb[rr];
            ref_model = result;
        end
    endfunction

    task check;
        input [127:0] din;
        input [127:0] exp;
        input [255:0] name;   // up to 32 chars
        begin
            state_in = din;
            #10;
            if (state_out === exp)
                $display("PASS: %0s  in=%h out=%h", name, din, state_out);
            else begin
                $display("FAIL: %0s  in=%h out=%h expected=%h", name, din, state_out, exp);
                errors = errors + 1;
            end
        end
    endtask

    initial begin
        errors = 0;

        // Test 1: bytes 00..0f (easy to read)
        check(128'h000102030405060708090a0b0c0d0e0f,
              128'h00050a0f04090e03080d02070c01060b, "sequential bytes");

        // Test 2: FIPS-197 Appendix B, round 1 (after SubBytes -> after ShiftRows)
        check(128'h63cab7040953d051cd60e0e7ba70e18c,
              128'h6353e08c0960e104cd70b751bacad0e7, "FIPS-197 round 1");

        // Test 3: all zeros / all ones (must be unchanged)
        check(128'h0, 128'h0, "all zeros");
        check({128{1'b1}}, {128{1'b1}}, "all ones");

        // Test 4: 200 random vectors vs reference model
        for (k = 0; k < 200; k = k + 1) begin
            state_in = {$random, $random, $random, $random};
            expected = ref_model(state_in);
            #10;
            if (state_out !== expected) begin
                $display("FAIL: random #%0d in=%h out=%h expected=%h",
                         k, state_in, state_out, expected);
                errors = errors + 1;
            end
        end
        $display("Random tests done.");

        if (errors == 0) $display("\n*** ALL TESTS PASSED ***\n");
        else             $display("\n*** %0d TEST(S) FAILED ***\n", errors);
        $finish;
    end
endmodule
