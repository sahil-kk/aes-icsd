#ifndef GF_MATH_H
#define GF_MATH_H

#include <stdint.h>

/**
 * Multiply an 8-bit polynomial in GF(2^8) by x (i.e. by 0x02).
 *
 * In AES literature, this fundamental operation is called "xtime".
 *
 * @param b  The 8-bit input value.
 * @return   The result of (b * 0x02) in GF(2^8).
 *
 * Hardware perspective:
 * This is a pure combinational logic circuit.
 * A left-shift by 1 in hardware costs 0 gates — it is just wiring input wire i
 * to output wire i+1. If the most significant bit (wire 7) is '1', an XOR
 * with the constant 0x1B (0001_1011 in binary) is applied using a few XOR gates.
 */
uint8_t gf_xtime(uint8_t b);

/**
 * Multiply two 8-bit elements in GF(2^8) modulo the AES irreducible
 * polynomial m(x) = x^8 + x^4 + x^3 + x + 1 (represented as 0x11B).
 *
 * Implemented bit-by-bit using shifts and XOR additions (no log tables).
 *
 * @param a  First 8-bit polynomial.
 * @param b  Second 8-bit polynomial.
 * @return   Product of a and b in GF(2^8).
 */
uint8_t gf_mul(uint8_t a, uint8_t b);

#endif /* GF_MATH_H */
