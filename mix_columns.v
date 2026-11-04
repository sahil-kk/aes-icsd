
module mix_columns (
    input  wire [7:0] state [0:3][0:3],
    output reg  [7:0] state_out [0:3][0:3]
);

    // Temporary column arrays
    reg [7:0] column_input [0:3];
    reg [7:0] column_output [0:3];

    integer col;

    // Mix one column
    task mix_single_column;
        input  [7:0] col_in [0:3];
        output [7:0] col_out [0:3];

        reg [7:0] s0, s1, s2, s3;
        reg [7:0] x0, x1, x2, x3;
        reg [7:0] mul3_s0, mul3_s1, mul3_s2, mul3_s3;

        begin
            s0 = col_in[0];
            s1 = col_in[1];
            s2 = col_in[2];
            s3 = col_in[3];

            // Multiplication by 02
            x0 = gf_xtime(s0);
            x1 = gf_xtime(s1);
            x2 = gf_xtime(s2);
            x3 = gf_xtime(s3);

            // Multiplication by 03
            mul3_s0 = x0 ^ s0;
            mul3_s1 = x1 ^ s1;
            mul3_s2 = x2 ^ s2;
            mul3_s3 = x3 ^ s3;

            // MixColumns equations
            col_out[0] = x0 ^ mul3_s1 ^ s2 ^ s3;
            col_out[1] = s0 ^ x1 ^ mul3_s2 ^ s3;
            col_out[2] = s0 ^ s1 ^ x2 ^ mul3_s3;
            col_out[3] = mul3_s0 ^ s1 ^ s2 ^ x3;
        end
    endtask

    // Apply MixColumns to all four columns
    integer i;

    always @(*) begin

        for (col = 0; col < 4; col = col + 1) begin

            column_input[0] = state[0][col];
            column_input[1] = state[1][col];
            column_input[2] = state[2][col];
            column_input[3] = state[3][col];

            mix_single_column(column_input, column_output);

            for (i = 0; i < 4; i = i + 1) begin
                state_out[i][col] = column_output[i];
            end

        end
    end

endmodule
