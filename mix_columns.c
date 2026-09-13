/**
 * ============================================================================
 * MODULE: mix_columns.c / mix_columns.h (AES MixColumns Transformation)
 * ============================================================================
 *
 * (a) WHAT THIS STAGE DOES IN PLAIN ENGLISH:
 *     MixColumns operates on the state column by column. It takes the 4 bytes
 *     in a column and treats them as a mathematical vector. It multiplies this
 *     vector by a fixed 4x4 matrix using Galois Field GF(2^8) arithmetic:
 *
 *       [ s'0 ]   [ 02  03  01  01 ]   [ s0 ]
 *       [ s'1 ] = [ 01  02  03  01 ] * [ s1 ]
 *       [ s'2 ]   [ 01  01  02  03 ]   [ s2 ]
 *       [ s'3 ]   [ 03  01  01  02 ]   [ s3 ]
 *
 *     The result is that every single byte in the output column depends on
 *     ALL FOUR bytes of the input column!
 *
 * (b) WHY AES NEEDS THIS STEP:
 *     This is the primary "diffusion" step of AES. In cryptography, diffusion
 *     means that if you change just a single bit of the plaintext, that change
 *     rapidly spreads like an avalanche to affect many bits across the entire
 *     state. Without MixColumns, changing one input byte would only ever affect
 *     that exact same byte throughout the rounds, leaving the cipher completely
 *     vulnerable to cryptanalysis. By combining ShiftRows (which moves bytes
 *     between columns) and MixColumns (which mixes bytes within a column),
 *     changing a single bit of plaintext affects all 128 bits of ciphertext after
 *     just 2 rounds!
 *
 * (c) HARDWARE PERSPECTIVE (BRIDGING TO VERILOG):
 *     - IS IT COMBINATIONAL OR SEQUENTIAL?
 *       PURE COMBINATIONAL.
 *
 *     - CIRCUIT STRUCTURE:
 *       Look at the matrix coefficients: 01, 02, and 03.
 *       - Multiplying by 01 is just passing the wire straight through: x
 *       - Multiplying by 02 is our simple xtime circuit: gf_xtime(x)
 *       - Multiplying by 03 is: gf_xtime(x) ^ x
 *       - Matrix addition is XOR (^).
 *
 *       Notice that there are NO complex multipliers or dividers needed!
 *       Each output byte is just a small tree of XOR gates connected to the
 *       input wires and a few xtime wire-shifts.
 *       In hardware, all 4 columns can be mixed simultaneously in parallel
 *       using 4 identical copies of this small combinational circuit block.
 *
 * ============================================================================
 */

#include "mix_columns.h"
#include "gf_math.h"

/**
 * Mix the 4 bytes of a single column using the AES matrix multiplication:
 *
 *   s'0 = (02 * s0) ^ (03 * s1) ^ (01 * s2) ^ (01 * s3)
 *   s'1 = (01 * s0) ^ (02 * s1) ^ (03 * s2) ^ (01 * s3)
 *   s'2 = (01 * s0) ^ (01 * s1) ^ (02 * s2) ^ (03 * s3)
 *   s'3 = (03 * s0) ^ (01 * s1) ^ (01 * s2) ^ (02 * s3)
 *
 * Hardware schematic equivalence:
 * Notice we do NOT compact this into a loop. Every single term is written out
 * explicitly so that when students design this module in Verilog, each line of C
 * corresponds directly to a wire or XOR gate in their schematic diagram.
 */
static void mix_single_column(uint8_t col_in[4], uint8_t col_out[4])
{
    uint8_t s0 = col_in[0];
    uint8_t s1 = col_in[1];
    uint8_t s2 = col_in[2];
    uint8_t s3 = col_in[3];

    /*
     * Precompute (02 * s) for each byte using the combinational gf_xtime unit.
     * Note: (03 * s) is computed as (02 * s) ^ s.
     */
    uint8_t x0 = gf_xtime(s0);       /* 02 * s0 */
    uint8_t x1 = gf_xtime(s1);       /* 02 * s1 */
    uint8_t x2 = gf_xtime(s2);       /* 02 * s2 */
    uint8_t x3 = gf_xtime(s3);       /* 02 * s3 */

    uint8_t mul3_s0 = x0 ^ s0;       /* 03 * s0 */
    uint8_t mul3_s1 = x1 ^ s1;       /* 03 * s1 */
    uint8_t mul3_s2 = x2 ^ s2;       /* 03 * s2 */
    uint8_t mul3_s3 = x3 ^ s3;       /* 03 * s3 */

    /*
     * Formulas written explicitly as an XOR tree:
     */
    /* Output Byte 0: (02 * s0) ^ (03 * s1) ^ (01 * s2) ^ (01 * s3) */
    col_out[0] = x0 ^ mul3_s1 ^ s2 ^ s3;

    /* Output Byte 1: (01 * s0) ^ (02 * s1) ^ (03 * s2) ^ (01 * s3) */
    col_out[1] = s0 ^ x1 ^ mul3_s2 ^ s3;

    /* Output Byte 2: (01 * s0) ^ (01 * s1) ^ (02 * s2) ^ (03 * s3) */
    col_out[2] = s0 ^ s1 ^ x2 ^ mul3_s3;

    /* Output Byte 3: (03 * s0) ^ (01 * s1) ^ (01 * s2) ^ (02 * s3) */
    col_out[3] = mul3_s0 ^ s1 ^ s2 ^ x3;
}

/**
 * Apply MixColumns to the entire 4x4 state matrix.
 *
 * In software, we iterate through the 4 columns: col = 0, 1, 2, 3.
 * In a parallel hardware design, 4 instances of mix_single_column execute
 * simultaneously in parallel across all columns.
 */
void mix_columns(uint8_t state[4][4])
{
    for (int col = 0; col < 4; col++) {
        uint8_t column_input[4];
        uint8_t column_output[4];

        /* Read column from state into input port buffer */
        column_input[0] = state[0][col];
        column_input[1] = state[1][col];
        column_input[2] = state[2][col];
        column_input[3] = state[3][col];

        /* Combinational column mixing */
        mix_single_column(column_input, column_output);

        /* Write output port buffer back into state */
        state[0][col] = column_output[0];
        state[1][col] = column_output[1];
        state[2][col] = column_output[2];
        state[3][col] = column_output[3];
    }
}
