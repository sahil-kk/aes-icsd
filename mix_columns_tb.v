
`timescale 1ns/1ps

module mix_columns_tb;

    reg  [7:0] state [0:3][0:3];
    wire [7:0] state_out [0:3][0:3];

    integer i, j;

    // Instantiate MixColumns
    mix_columns uut (
        .state(state),
        .state_out(state_out)
    );

    initial begin

        $display("AES MIXCOLUMNS TESTBENCH");
        $display("------------------------");

        // Initialize all state bytes to zero
        for (i = 0; i < 4; i = i + 1) begin
            for (j = 0; j < 4; j = j + 1) begin
                state[i][j] = 8'h00;
            end
        end

        // Test 1: Standard AES test vector
        state[0][0] = 8'hDB;
        state[1][0] = 8'h13;
        state[2][0] = 8'h53;
        state[3][0] = 8'h45;

        #10;

        $display("TEST 1");
        $display("Input  : DB 13 53 45");
        $display("Output : %h %h %h %h",
            state_out[0][0],
            state_out[1][0],
            state_out[2][0],
            state_out[3][0]
        );

        if (state_out[0][0] === 8'h8E &&
            state_out[1][0] === 8'h4D &&
            state_out[2][0] === 8'hA1 &&
            state_out[3][0] === 8'hBC)
            $display("TEST 1 PASSED");
        else
            $display("TEST 1 FAILED");

        // Test 2: All-zero input
        for (i = 0; i < 4; i = i + 1) begin
            for (j = 0; j < 4; j = j + 1) begin
                state[i][j] = 8'h00;
            end
        end

        #10;

        $display("TEST 2");

        if (state_out[0][0] === 8'h00 &&state_out[1][0] === 8'h00 &&state_out[2][0] === 8'h00 &&state_out[3][0] === 8'h00)
            $display("TEST 2 PASSED");
        else
            $display("TEST 2 FAILED");

        $display("------------------------");

        $finish;

    end

endmodule

