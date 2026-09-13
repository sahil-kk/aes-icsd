/**
 * ============================================================================
 * MODULE: key_expansion.c / key_expansion.h (AES-128 Key Schedule)
 * ============================================================================
 *
 * (a) WHAT THIS STAGE DOES IN PLAIN ENGLISH:
 *     AES-128 starts with a single 128-bit (16-byte) master secret key.
 *     However, the AES encryption algorithm runs for 10 rounds, plus an initial
 *     pre-round step, meaning it requires 11 distinct 128-bit keys (called
 *     "Round Keys"). Key Expansion is the mathematical recipe that stretches
 *     the original 16-byte key into a total of 176 bytes (11 keys x 16 bytes).
 *     Each new round key is derived from the previous round key using byte
 *     rotation, S-box substitution, and round constant (Rcon) XORing.
 *
 * (b) WHY AES NEEDS THIS STEP:
 *     If AES used the exact same secret key in every single round, an attacker
 *     could exploit the repetitive symmetry of the rounds to break the cipher
 *     using slide attacks. Generating a unique, pseudo-random round key for
 *     each round breaks mathematical symmetry and ensures that every round
 *     transforms the data in a fundamentally distinct way.
 *
 * (c) HARDWARE PERSPECTIVE (BRIDGING TO VERILOG):
 *     In hardware, this step naturally happens "over time" — one round key per
 *     clock cycle — since each new key depends on the previous one.
 *
 *     - CONCEPT: SEQUENTIAL LOGIC
 *       Sequential logic is digital logic whose output depends on a stored
 *       previous result (stored in physical memory cells called registers or
 *       flip-flops), not just current inputs.
 *
 *     - WHY IT MATTERS HERE:
 *       Notice how Round Key 1 cannot be calculated until Round Key 0 is ready.
 *       Round Key 2 cannot be calculated until Round Key 1 is ready, and so on.
 *       In hardware design, rather than building 10 separate key generator circuits
 *       (which would consume huge chip area), engineers typically build ONE key
 *       generator circuit connected to a register. On Clock Cycle 1, it computes
 *       Round Key 1 and stores it; on Clock Cycle 2, it feeds that stored value
 *       back into the circuit to compute Round Key 2, and so on.
 *
 * ============================================================================
 */

#include "key_expansion.h"
#include "sbox.h"
#include "rcon.h"

/**
 * Helper: Rotate a 4-byte column (word) cyclically upward by 1 byte.
 * Input : [b0, b1, b2, b3]
 * Output: [b1, b2, b3, b0]
 *
 * Hardware perspective:
 * This requires ZERO logic gates. It is just cross-wiring:
 * wire 0 connects to position 3, wire 1 to position 0, etc.
 */
static void rot_word(const uint8_t in_word[4], uint8_t out_word[4])
{
    out_word[0] = in_word[1];
    out_word[1] = in_word[2];
    out_word[2] = in_word[3];
    out_word[3] = in_word[0];
}

/**
 * Helper: Apply the S-box substitution table to each of the 4 bytes in a word.
 * Input : [b0, b1, b2, b3]
 * Output: [S(b0), S(b1), S(b2), S(b3)]
 *
 * Hardware perspective:
 * In hardware, this requires 4 S-box lookup ROMs operating in parallel.
 */
static void sub_word(const uint8_t in_word[4], uint8_t out_word[4])
{
    out_word[0] = sbox_lookup(in_word[0]);
    out_word[1] = sbox_lookup(in_word[1]);
    out_word[2] = sbox_lookup(in_word[2]);
    out_word[3] = sbox_lookup(in_word[3]);
}

/**
 * Expand 16-byte cipher key into 11 round keys (each 4x4 bytes).
 *
 * Memory layout:
 * Hardware does not have dynamic memory (no heap, no malloc/free). Everything
 * is mapped to fixed physical registers and wires. Hence, we store all 11 round
 * keys in a fixed-size 3D array: round_keys[11][4][4], where:
 *   - Dimension 1 (index 0..10): Round number
 *   - Dimension 2 (index 0..3) : Row index in 4x4 matrix
 *   - Dimension 3 (index 0..3) : Column index in 4x4 matrix
 */
void aes128_key_expansion(const uint8_t key[16], uint8_t round_keys[11][4][4])
{
    /* ------------------------------------------------------------------------
     * ROUND 0 KEY:
     * The very first round key (Round 0) is simply the master key itself,
     * copied directly into the 4x4 matrix column by column:
     *   col 0 = bytes 0..3
     *   col 1 = bytes 4..7
     *   col 2 = bytes 8..11
     *   col 3 = bytes 12..15
     * ------------------------------------------------------------------------ */
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            round_keys[0][row][col] = key[row + (4 * col)];
        }
    }

    /* ------------------------------------------------------------------------
     * ROUNDS 1 TO 10:
     * We generate one round key per iteration. In hardware, each iteration of
     * this loop corresponds to 1 clock cycle.
     * ------------------------------------------------------------------------ */
    for (int round_num = 1; round_num <= 10; round_num++) {
        /*
         * Extract the last column (Column 3) of the PREVIOUS round key.
         * In AES terminology, this 4-byte column is word W[4*round - 1].
         */
        uint8_t prev_col3[4];
        for (int row = 0; row < 4; row++) {
            prev_col3[row] = round_keys[round_num - 1][row][3];
        }

        /* Step 1: Rotate the 4-byte column */
        uint8_t rotated[4];
        rot_word(prev_col3, rotated);

        /* Step 2: Substitute each byte through the S-box */
        uint8_t substituted[4];
        sub_word(rotated, substituted);

        /*
         * Step 3: XOR the first byte with the Round Constant Rcon[round_num].
         * The remaining 3 bytes are XORed with 0x00 (meaning they stay unchanged).
         */
        uint8_t transformed[4];
        transformed[0] = substituted[0] ^ Rcon[round_num];
        transformed[1] = substituted[1] ^ 0x00;
        transformed[2] = substituted[2] ^ 0x00;
        transformed[3] = substituted[3] ^ 0x00;

        /*
         * Step 4: Generate the 4 columns of the new round key using chaining:
         *
         * Column 0 = Column 0 of (round - 1) XOR transformed
         * Column 1 = Column 1 of (round - 1) XOR Column 0 of this round
         * Column 2 = Column 2 of (round - 1) XOR Column 1 of this round
         * Column 3 = Column 3 of (round - 1) XOR Column 2 of this round
         */

        /* Column 0 */
        for (int row = 0; row < 4; row++) {
            round_keys[round_num][row][0] = round_keys[round_num - 1][row][0] ^ transformed[row];
        }

        /* Column 1 */
        for (int row = 0; row < 4; row++) {
            round_keys[round_num][row][1] = round_keys[round_num - 1][row][1] ^ round_keys[round_num][row][0];
        }

        /* Column 2 */
        for (int row = 0; row < 4; row++) {
            round_keys[round_num][row][2] = round_keys[round_num - 1][row][2] ^ round_keys[round_num][row][1];
        }

        /* Column 3 */
        for (int row = 0; row < 4; row++) {
            round_keys[round_num][row][3] = round_keys[round_num - 1][row][3] ^ round_keys[round_num][row][2];
        }
    }
}
