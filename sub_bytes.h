#ifndef SUB_BYTES_H
#define SUB_BYTES_H

#include <stdint.h>

/**
 * Apply the non-linear S-box substitution independently to all 16 bytes of
 * the 4x4 state matrix.
 *
 * @param state  In/Out 4x4 AES state matrix.
 *
 * Hardware perspective:
 * This module illustrates a classic hardware design trade-off:
 * - High-speed / Large Area: 16 parallel S-Box ROMs, computes all 16 bytes in 1 cycle.
 * - Low-resource / Small Area: 1 shared S-Box ROM, computes 1 byte per cycle over 16 cycles.
 */
void sub_bytes(uint8_t state[4][4]);

#endif /* SUB_BYTES_H */
