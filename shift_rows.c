/**
 * ============================================================================
 * MODULE: shift_rows.c / shift_rows.h (AES ShiftRows Transformation)
 * ============================================================================
 *
 * (a) WHAT THIS STAGE DOES IN PLAIN ENGLISH:
 *     ShiftRows cyclically shifts (rotates) the bytes in each row of the 4x4
 *     state matrix to the left:
 *       - Row 0 is shifted by 0 positions (left untouched).
 *       - Row 1 is shifted left by 1 position.
 *       - Row 2 is shifted left by 2 positions.
 *       - Row 3 is shifted left by 3 positions.
 *
 * (b) WHY AES NEEDS THIS STEP:
 *     In AES, data is arranged in a 4x4 matrix. The next step (MixColumns) only
 *     mixes bytes within the *same* column. If we never moved bytes between
 *     columns, each 4-byte column would be encrypted completely independently
 *     of the other three columns (like 4 tiny, weak ciphers side-by-side).
 *     ShiftRows diffuses data across columns so that bytes from different
 *     columns get mixed together in subsequent rounds.
 *
 * (c) HARDWARE PERSPECTIVE (BRIDGING TO VERILOG):
 *     In hardware, this stage requires NO logic gates at all — it is literally
 *     just rewiring which output connects to which input!
 *
 *     - IS IT COMBINATIONAL OR SEQUENTIAL?
 *       It is not even combinational logic in the sense of gates — it is pure
 *       INTERCONNECT (physical copper wire traces on silicon).
 *
 *     - WHY THIS IS A CRITICAL HARDWARE INSIGHT:
 *       In C software, moving memory around takes instructions, temporary
 *       variables, and CPU clock cycles. In hardware, wires are physically
 *       fixed: if output wire A goes to input pin B, data moves instantly at the
 *       speed of light with zero delay, zero transistors, and zero power.
 *       ShiftRows is therefore "completely free" in hardware.
 *
 * ============================================================================
 */

#include "shift_rows.h"

/**
 * Perform cyclic row rotation on the 4x4 state matrix.
 *
 * We write each row's shift literally (0, 1, 2, 3) without clever mathematical
 * loops or modulo arithmetic, directly mirroring the physical wire connections
 * in a hardware schematic.
 */
void shift_rows(uint8_t state[4][4])
{
    /* ------------------------------------------------------------------------
     * ROW 0: Shift by 0 bytes (No change)
     * In hardware: state_out[0] wires directly connect to state_in[0] wires.
     * ------------------------------------------------------------------------ */
    /* state[0][0] = state[0][0]; */
    /* state[0][1] = state[0][1]; */
    /* state[0][2] = state[0][2]; */
    /* state[0][3] = state[0][3]; */

    /* ------------------------------------------------------------------------
     * ROW 1: Shift left by 1 byte
     * [s0, s1, s2, s3]  --->  [s1, s2, s3, s0]
     * ------------------------------------------------------------------------ */
    {
        uint8_t temp = state[1][0];
        state[1][0] = state[1][1];
        state[1][1] = state[1][2];
        state[1][2] = state[1][3];
        state[1][3] = temp;
    }

    /* ------------------------------------------------------------------------
     * ROW 2: Shift left by 2 bytes
     * [s0, s1, s2, s3]  --->  [s2, s3, s0, s1]
     * ------------------------------------------------------------------------ */
    {
        uint8_t temp0 = state[2][0];
        uint8_t temp1 = state[2][1];
        state[2][0] = state[2][2];
        state[2][1] = state[2][3];
        state[2][2] = temp0;
        state[2][3] = temp1;
    }

    /* ------------------------------------------------------------------------
     * ROW 3: Shift left by 3 bytes (equivalent to shifting right by 1 byte)
     * [s0, s1, s2, s3]  --->  [s3, s0, s1, s2]
     * ------------------------------------------------------------------------ */
    {
        uint8_t temp = state[3][3];
        state[3][3] = state[3][2];
        state[3][2] = state[3][1];
        state[3][1] = state[3][0];
        state[3][0] = temp;
    }
}
