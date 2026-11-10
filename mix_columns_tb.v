
`timescale 1ns/1ps

module mix_columns_tb;

    reg  [127:0] state_in;
    wire [127:0] state_out;

    // Instantiate MixColumns module
    mix_columns uut (
        .state_in(state_in),
        .state_out(state_out)
    );

    initial begin

        $display("AES-128 MixColumns Testbench");
        $display("----------------------------");

        // Test 1: Standard AES test vector
        // Four columns are tested independently.

        state_in = {
            32'hD4BF5D30,
            32'hE0B452AE,
            32'hB84111F1,
            32'h1E2798E5
        };

        #10;

        $display("Input  : %h", state_in);
        $display("Output : %h", state_out);

        if (state_out[127:96] == 32'h046681E5)
            $display("Column 0: PASS");
        else
            $display("Column 0: FAIL");

        #10;

        // Test 2: All-zero input
        state_in = 128'h0;

        #10;

        if (state_out == 128'h0)
            $display("Zero test: PASS");
        else
            $display("Zero test: FAIL");

        #10;

        // Test 3: All-one bytes
        state_in = {16{8'hFF}};

        #10;

        $display("All FF Input  : %h", state_in);
        $display("All FF Output : %h", state_out);

        $display("----------------------------");
        $display("Simulation completed.");

        $finish;
    end

endmodule
