`timescale 1ns / 1ps

module tb_sub_bytes;

    reg  [127:0] state_in;
    wire [127:0] state_out;

    sub_bytes uut (
        .state_in(state_in),
        .state_out(state_out)
    );

    initial begin

        $display("AES SubBytes Testbench");

        state_in = 128'h1953207c112233445566778899aabbcc;

        #10;

        $display("Input State : %h", state_in);
        $display("Output State: %h", state_out);

        if (state_out === 128'hd4edb71082fc931b63c333ac10f5eaea)
            $display("TEST PASSED");
        else
            $display("TEST FAILED");

        $finish;

    end

endmodule
