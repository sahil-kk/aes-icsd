`timescale 1ns/1ps
// Testbench: checks all 11 round keys for two known vectors.
module tb_key_expansion;
    reg          clk = 0, rst_n = 0, start = 0;
    reg  [127:0] key;
    wire [1407:0] all_keys;
    wire         ready;

    key_expansion dut (.clk(clk), .rst_n(rst_n), .start(start),
                       .key(key), .all_keys(all_keys), .ready(ready));

    always #5 clk = ~clk;

    integer errors = 0;
    integer r;
    reg [127:0] exp [0:10];
    reg [127:0] got;

    task run_check;
        begin
            @(negedge clk); start = 1;
            @(negedge clk); start = 0;
            wait (ready);
            @(negedge clk);
            for (r = 0; r <= 10; r = r + 1) begin
                got = all_keys[1407 - 128*r -: 128];
                if (got !== exp[r]) begin
                    $display("FAIL round %0d: got %h exp %h", r, got, exp[r]);
                    errors = errors + 1;
                end else
                    $display("ok   round %0d: %h", r, got);
            end
        end
    endtask

    initial begin
        // FIPS-197 Appendix A.1 / key 2b7e1516...
        key = 128'h2b7e151628aed2a6abf7158809cf4f3c;
        exp[0]  = 128'h2b7e151628aed2a6abf7158809cf4f3c;
        exp[1]  = 128'ha0fafe1788542cb123a339392a6c7605;
        exp[2]  = 128'hf2c295f27a96b9435935807a7359f67f;
        exp[3]  = 128'h3d80477d4716fe3e1e237e446d7a883b;
        exp[4]  = 128'hef44a541a8525b7fb671253bdb0bad00;
        exp[5]  = 128'hd4d1c6f87c839d87caf2b8bc11f915bc;
        exp[6]  = 128'h6d88a37a110b3efddbf98641ca0093fd;
        exp[7]  = 128'h4e54f70e5f5fc9f384a64fb24ea6dc4f;
        exp[8]  = 128'head27321b58dbad2312bf5607f8d292f;
        exp[9]  = 128'hac7766f319fadc2128d12941575c006e;
        exp[10] = 128'hd014f9a8c9ee2589e13f0cc8b6630ca6;

        #22 rst_n = 1;
        $display("--- Vector 1: key 2b7e1516... ---");
        run_check;

        // FIPS-197 Appendix C.1 / key 000102...0f
        key = 128'h000102030405060708090a0b0c0d0e0f;
        exp[0]  = 128'h000102030405060708090a0b0c0d0e0f;
        exp[1]  = 128'hd6aa74fdd2af72fadaa678f1d6ab76fe;
        exp[2]  = 128'hb692cf0b643dbdf1be9bc5006830b3fe;
        exp[3]  = 128'hb6ff744ed2c2c9bf6c590cbf0469bf41;
        exp[4]  = 128'h47f7f7bc95353e03f96c32bcfd058dfd;
        exp[5]  = 128'h3caaa3e8a99f9deb50f3af57adf622aa;
        exp[6]  = 128'h5e390f7df7a69296a7553dc10aa31f6b;
        exp[7]  = 128'h14f9701ae35fe28c440adf4d4ea9c026;
        exp[8]  = 128'h47438735a41c65b9e016baf4aebf7ad2;
        exp[9]  = 128'h549932d1f08557681093ed9cbe2c974e;
        exp[10] = 128'h13111d7fe3944a17f307a78b4d2b30c5;

        $display("--- Vector 2: key 000102...0f ---");
        run_check;

        if (errors == 0) $display("[RESULT] PASS");
        else             $display("[RESULT] FAIL (%0d errors)", errors);
        $finish;
    end
endmodule
