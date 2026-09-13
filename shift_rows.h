#ifndef SHIFT_ROWS_H
#define SHIFT_ROWS_H

#include <stdint.h>

/**
 * Perform the ShiftRows transformation: cyclically shift the bytes in each row
 * of the 4x4 state matrix to the left by an offset equal to the row index (0, 1, 2, 3).
 *
 * @param state  In/Out 4x4 AES state matrix.
 *
 * Hardware perspective:
 * This operation is "FREE" in hardware! It requires ZERO logic gates, ZERO clock
 * cycles, and ZERO power. In Verilog, it is implemented purely by routing wires:
 * connecting output pins of one stage to different input pins of the next stage.
 */
void shift_rows(uint8_t state[4][4]);

#endif /* SHIFT_ROWS_H */
