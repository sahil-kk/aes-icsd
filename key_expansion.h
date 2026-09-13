#ifndef KEY_EXPANSION_H
#define KEY_EXPANSION_H

#include <stdint.h>

/**
 * Expand a 128-bit (16-byte) master cipher key into 11 distinct 128-bit round keys.
 *
 * @param key         Input master key: 16 bytes.
 * @param round_keys  Output expanded key schedule: 11 round keys, each 4x4 bytes.
 *                    round_keys[r][row][col] holds byte at (row, col) for round r.
 *
 * Hardware perspective:
 * In a pipelined or iterative hardware AES core, key expansion is implemented
 * using sequential logic: on each rising edge of the clock signal, one new 128-bit
 * round key is computed and stored into a register bank.
 */
void aes128_key_expansion(const uint8_t key[16], uint8_t round_keys[11][4][4]);

#endif /* KEY_EXPANSION_H */
