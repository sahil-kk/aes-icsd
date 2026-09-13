/**
 * ============================================================================
 * MODULE: sub_bytes.c / sub_bytes.h (AES SubBytes Transformation)
 * ============================================================================
 *
 * (a) WHAT THIS STAGE DOES IN PLAIN ENGLISH:
 *     SubBytes takes every single byte in the 4x4 state matrix (16 bytes in total)
 *     and independently replaces it with its corresponding byte from the S-box
 *     lookup table. If state[0][0] is 0x19, it looks up 0x19 in the S-box and
 *     replaces state[0][0] with 0xd4.
 *
 * (b) WHY AES NEEDS THIS STEP:
 *     As discussed in sbox.c, SubBytes is the non-linear "confusion" step of AES.
 *     It scrambles the mathematical relationship between the plaintext and the
 *     ciphertext. Without this non-linear step, the entire AES cipher could be
 *     solved as a set of linear equations.
 *
 * (c) HARDWARE PERSPECTIVE (BRIDGING TO VERILOG):
 *     This stage introduces one of the most fundamental concepts in digital
 *     circuit design: the ARCHITECTURAL TRADE-OFF (Speed vs. Chip Area).
 *
 *     In hardware, you have choices that do not exist in standard software:
 *     1. FULLY PARALLEL ARCHITECTURE (Maximum Speed):
 *        Instantiate 16 independent S-box ROM circuits side-by-side on the silicon.
 *        All 16 bytes are substituted simultaneously in parallel within a fraction
 *        of a single clock cycle!
 *        Drawback: Consumes 16 times as much silicon area (16 x 256-byte ROMs).
 *
 *     2. ITERATIVE / SERIAL ARCHITECTURE (Minimum Silicon Area):
 *        Instantiate only 1 S-box ROM on the chip. Use multiplexers and a 4-bit
 *        counter to feed one byte of the state into the S-box per clock cycle,
 *        taking 16 clock cycles to complete the entire state.
 *        Benefit: Saves massive silicon area on constrained devices (e.g. smartcards).
 *        Drawback: Takes 16 clock cycles instead of 1.
 *
 *     When you later implement AES in Verilog, you will explicitly choose which
 *     trade-off best fits your system requirements!
 *
 * ============================================================================
 */

#include "sub_bytes.h"
#include "sbox.h"

/**
 * Apply S-Box substitution to each byte of the 4x4 state.
 *
 * In software, we loop through rows and columns.
 * In a parallel hardware implementation, 16 sbox_lookup modules are wired
 * simultaneously to the 16 bytes of the state register.
 */
void sub_bytes(uint8_t state[4][4])
{
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            state[row][col] = sbox_lookup(state[row][col]);
        }
    }
}
