`timescale 1ns / 1ps

module tb_add_round_key;

    // Testbench signals (128-bit buses)
    reg  [127:0] state_in;
    reg  [127:0] round_key;
    wire [127:0] state_out;

    // Instantiate the Unit Under Test (UUT)
    add_round_key uut (
        .state_in  (state_in),
        .round_key (round_key),
        .state_out (state_out)
    );

    // Task to apply vectors and verify output
    task run_test(
        input [127:0] test_state,
        input [127:0] test_key,
        input [127:0] expected_out,
        input [8*25:1] test_name
    );
    begin
        state_in  = test_state;
        round_key = test_key;
        #10; // Wait 10 ns for combinational propagation

        if (state_out === expected_out) begin
            $display("[PASS] %s", test_name);
            $display("       state_in  = %h", state_in);
            $display("       round_key = %h", round_key);
            $display("       state_out = %h\n", state_out);
        end else begin
            $display("[FAIL] %s", test_name);
            $display("       Expected: %h", expected_out);
            $display("       Got:      %h\n", state_out);
        end
    end
    endtask

    initial begin
        // Optional waveform dump for GTKWave / ModelSim / Vivado
        $dumpfile("tb_add_round_key.vcd");
        $dumpvars(0, tb_add_round_key);

        $display("==========================================================");
        $display("         Starting AES AddRoundKey Testbench               ");
        $display("==========================================================\n");

        // Test 1: NIST FIPS-197 Appendix B Initial AddRoundKey vector
        run_test(
            128'h3243f6a8_885a308d_313198a2_e0370734,
            128'h2b7e1516_28aed2a6_abf71588_09cf4f3c,
            128'h193de3be_a0f4e22b_9ac68d2a_e9f84808,
            "FIPS-197 Appendix B"
        );

        // Test 2: NIST FIPS-197 Appendix C.1 (AES-128) Initial Round
        run_test(
            128'h00112233_44556677_8899aabb_ccddeeff,
            128'h00010203_04050607_08090a0b_0c0d0e0f,
            128'h00102030_40506070_8090a0b0_c0d0e0f0,
            "FIPS-197 Appendix C.1"
        );

        // Test 3: Identity property (XOR with all zeros leaves state unchanged)
        run_test(
            128'hdeadbeef_cafebabe_01234567_89abcdef,
            128'h00000000_00000000_00000000_00000000,
            128'hdeadbeef_cafebabe_01234567_89abcdef,
            "Zero Key Identity"
        );

        // Test 4: Self-inverse property (XOR with identical value yields zero)
        run_test(
            128'hffffffff_ffffffff_ffffffff_ffffffff,
            128'hffffffff_ffffffff_ffffffff_ffffffff,
            128'h00000000_00000000_00000000_00000000,
            "All-Ones Self-Inverse"
        );

        $display("==========================================================");
        $display("                 Simulation Complete                      ");
        $display("==========================================================");
        $finish;
    end

endmodule
