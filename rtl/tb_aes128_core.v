`timescale 1ns/1ps

// Testbench for the Verilog-2001 AES-128 iterative encryption core.
// Uses the FIPS-197 AES-128 known-answer test vector.

module tb_aes128_core;

    reg          clk;
    reg          rst_n;
    reg          start;
    reg  [127:0] plaintext;
    reg  [127:0] key;

    wire [127:0] ciphertext;
    wire         busy;
    wire         done;

    reg  [127:0] expected_ciphertext;
    integer      cycle_count;

    // Device under test
    aes128_core dut (
        .clk        (clk),
        .rst_n      (rst_n),
        .start      (start),
        .plaintext  (plaintext),
        .key        (key),
        .ciphertext (ciphertext),
        .busy       (busy),
        .done       (done)
    );

    // 100 MHz simulation clock, period = 10 ns.
    initial begin
        clk = 1'b0;
        forever #5 clk = ~clk;
    end

    initial begin

        $dumpfile("aes_wave.vcd");
        $dumpvars(0, tb_aes128_core);
        
        // FIPS-197 Appendix C.1 AES-128 example.
        plaintext          = 128'h00112233445566778899aabbccddeeff;
        key                = 128'h000102030405060708090a0b0c0d0e0f;
        expected_ciphertext= 128'h69c4e0d86a7b0430d8cdb78070b4c55a;

        rst_n       = 1'b0;
        start       = 1'b0;
        cycle_count = 0;

        // Hold reset for two rising edges.
        repeat (2) @(posedge clk);
        @(negedge clk);
        rst_n = 1'b1;

        // Apply a one-cycle start pulse.
        @(negedge clk);
        start = 1'b1;
        @(negedge clk);
        start = 1'b0;

        // Wait for completion, with a timeout to catch an FSM hang.
        // Sample on negedge clk to safely catch the 1-cycle done pulse.
        while (done !== 1'b1 && cycle_count < 30) begin
            @(negedge clk);
            cycle_count = cycle_count + 1;
        end

        // Allow nonblocking assignments to update before checking outputs.
        #1;

        if (done !== 1'b1) begin
            $display("FAIL: timeout, AES core did not assert done.");
            $display("busy=%b done=%b", busy, done);
            $finish;
        end

        $display("Plaintext : %032h", plaintext);
        $display("Key       : %032h", key);
        $display("Expected  : %032h", expected_ciphertext);
        $display("Received  : %032h", ciphertext);

        if (ciphertext === expected_ciphertext) begin
            $display("PASS: AES-128 known-answer test passed.");
        end else begin
            $display("FAIL: AES-128 ciphertext mismatch.");
        end

        $finish;
    end

endmodule
