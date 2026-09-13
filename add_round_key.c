/**
 * ============================================================================
 * MODULE: add_round_key.c / add_round_key.h (AES AddRoundKey Transformation)
 * ============================================================================
 *
 * (a) WHAT THIS STAGE DOES IN PLAIN ENGLISH:
 *     This stage takes the 16 bytes of data currently being encrypted (the "state")
 *     and combines them with the 16 bytes of the current round's key.
 *     It does this using the bitwise XOR operation (`^`), byte-by-byte:
 *         state[row][col] = state[row][col] ^ round_key[row][col]
 *
 * (b) WHY AES NEEDS THIS STEP:
 *     This is the ONLY place in the entire AES algorithm where the secret key
 *     is actually blended into the data! Without AddRoundKey, all the other stages
 *     (SubBytes, ShiftRows, MixColumns) are completely unkeyed, meaning anyone
 *     who knows the algorithm could reverse them without needing any secret key at all.
 *     AddRoundKey injects the secret entropy into the block.
 *
 * (c) HARDWARE PERSPECTIVE (BRIDGING TO VERILOG):
 *     This is the simplest possible hardware stage in AES:
 *
 *     - IS IT COMBINATIONAL OR SEQUENTIAL?
 *       PURE COMBINATIONAL. There is no memory, no clock, and no state machine.
 *
 *     - HARDWARE IMPLEMENTATION:
 *       In C, we write two nested for-loops (4 rows x 4 columns = 16 iterations)
 *       because a software CPU executes instructions sequentially one at a time.
 *       In hardware (Verilog), all 16 byte XOR operations happen AT THE EXACT
 *       SAME INSTANT! You have 128 input wires for the state, 128 input wires
 *       for the round key, and 128 two-input XOR gates operating in parallel.
 *       The result appears at the 128 output wires after just a fraction of a
 *       nanosecond.
 *
 *       In Verilog, this whole C file translates to a single continuous assignment:
 *           assign state_out = state_in ^ round_key;
 *
 * ============================================================================
 */

#include "add_round_key.h"

/**
 * AddRoundKey: XOR state with round key.
 *
 * Notice the function signature:
 * - uint8_t state[4][4] is modified in-place (acting as input AND output bus).
 * - const uint8_t round_key[4][4] is read-only (acting as secondary input bus).
 * No global variables are touched.
 */
void add_round_key(uint8_t state[4][4], const uint8_t round_key[4][4])
{
    /*
     * Explicit 4x4 nested loop over all 16 bytes.
     * In hardware, these 16 iterations execute simultaneously in parallel.
     */
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            state[row][col] = state[row][col] ^ round_key[row][col];
        }
    }
}
