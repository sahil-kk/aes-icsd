#ifndef ADD_ROUND_KEY_H
#define ADD_ROUND_KEY_H

#include <stdint.h>

/**
 * Perform the AddRoundKey transformation: bitwise XOR of the 4x4 state matrix
 * with the 4x4 round key matrix.
 *
 * @param state      In/Out 4x4 AES state matrix (modified in-place).
 * @param round_key  Input 4x4 round key matrix for the current round.
 *
 * Hardware perspective:
 * This is the simplest possible hardware module in AES: 16 parallel 8-bit XOR
 * gates (128 single-bit XOR gates total) operating simultaneously in a single
 * combinational step with minimal propagation delay.
 */
void add_round_key(uint8_t state[4][4], const uint8_t round_key[4][4]);

#endif /* ADD_ROUND_KEY_H */
