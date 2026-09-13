#ifndef MIX_COLUMNS_H
#define MIX_COLUMNS_H

#include <stdint.h>

/**
 * Perform the MixColumns transformation: multiply each 4-byte column of the
 * 4x4 state matrix by a fixed polynomial matrix over GF(2^8).
 *
 * @param state  In/Out 4x4 AES state matrix.
 *
 * Hardware perspective:
 * This module is a purely combinational circuit composed of gf_xtime modules
 * and XOR trees. Because each column is processed independently, hardware can
 * instantiate 4 identical column mixing circuits that operate simultaneously
 * in parallel in a single clock cycle.
 */
void mix_columns(uint8_t state[4][4]);

#endif /* MIX_COLUMNS_H */
