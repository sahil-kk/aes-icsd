#ifndef RCON_H
#define RCON_H

#include <stdint.h>

/**
 * AES Round Constants (Rcon).
 *
 * In AES-128 key expansion, a round constant is XORed with the first column
 * of each round key to eliminate symmetry in the key schedule.
 *
 * Rcon[i] contains the powers of x (0x02) in GF(2^8):
 *   Rcon[1]  = 0x01 (x^0)
 *   Rcon[2]  = 0x02 (x^1)
 *   Rcon[3]  = 0x04 (x^2)
 *   Rcon[4]  = 0x08 (x^3)
 *   Rcon[5]  = 0x10 (x^4)
 *   Rcon[6]  = 0x20 (x^5)
 *   Rcon[7]  = 0x40 (x^6)
 *   Rcon[8]  = 0x80 (x^7)
 *   Rcon[9]  = 0x1B (x^8 mod m(x) = 0x11B)
 *   Rcon[10] = 0x36 (x^9 mod m(x) = 0x1B * 2)
 *
 * Index 0 is unused (set to 0x00) so that Rcon[round] matches the 1-based
 * round number (rounds 1 through 10).
 *
 * Hardware perspective:
 * This is another small lookup table (ROM) or hardwired 4-bit decoder.
 * Since Rcon is only 10 bytes, in hardware it is often implemented as a
 * simple multiplexer or small state register that shifts left each round.
 */
static const uint8_t Rcon[11] = {
    0x00, /* round 0 (unused) */
    0x01, /* round 1 */
    0x02, /* round 2 */
    0x04, /* round 3 */
    0x08, /* round 4 */
    0x10, /* round 5 */
    0x20, /* round 6 */
    0x40, /* round 7 */
    0x80, /* round 8 */
    0x1B, /* round 9 */
    0x36  /* round 10 */
};

#endif /* RCON_H */
