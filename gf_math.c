/**
 * ============================================================================
 * MODULE: gf_math.c / gf_math.h (Galois Field GF(2^8) Arithmetic)
 * ============================================================================
 *
 * (a) WHAT THIS STAGE DOES IN PLAIN ENGLISH:
 *     This module performs multiplication on bytes using special rules known as
 *     Galois Field (or Finite Field) arithmetic, denoted as GF(2^8).
 *     Instead of treating a byte as a regular number (0 to 255) where 200 * 2 = 400
 *     (which overflows 8 bits), we treat each byte as a polynomial with 8 binary
 *     coefficients: b7*x^7 + b6*x^6 + ... + b1*x + b0.
 *     Addition is just bitwise XOR (no carries!).
 *     Multiplication is polynomial multiplication, and whenever the degree reaches
 *     x^8, we divide by an irreducible "reduction polynomial" and keep the remainder:
 *         m(x) = x^8 + x^4 + x^3 + x + 1  (in hex: 0x11B)
 *
 * (b) WHY AES NEEDS THIS STEP:
 *     Normal computer math overflows: if you multiply two 8-bit numbers, you need
 *     16 bits to store the result, or if you truncate, you lose mathematical
 *     reversibility (e.g. 128 * 2 mod 256 = 0, so you can't divide by 2 to get back 128).
 *     In GF(2^8), every non-zero element has a unique multiplicative inverse, every
 *     operation stays strictly bounded to 1 byte (0 to 255), and no information is
 *     ever lost! This mathematical property allows the MixColumns stage of AES to
 *     thoroughly scramble bytes while guaranteeing that decryption can cleanly
 *     reverse the scrambling.
 *
 * (c) HARDWARE PERSPECTIVE (BRIDGING TO VERILOG):
 *     Each shift and XOR here corresponds to a wire connection and an XOR gate in
 *     hardware — this function could become a small combinational circuit with no
 *     memory needed.
 *
 *     - IS IT COMBINATIONAL OR SEQUENTIAL?
 *       COMBINATIONAL. There are no clock edges, no registers, and no memory.
 *       - In hardware, a left-shift `<< 1` requires ZERO gates: you literally take
 *         wire [6:0] and connect them to output pins [7:1], wiring output pin 0 to ground!
 *       - Addition in GF(2^8) is XOR: each bit requires exactly one 2-input XOR gate.
 *       - Testing the high bit `(b & 0x80)` is just looking at wire [7]. If wire [7]
 *         is 1, we XOR with 0x1B; otherwise we XOR with 0x00. In digital logic, this
 *         is realized with a simple 2-to-1 multiplexer or an AND gate enabling the XOR.
 *
 * ============================================================================
 */

#include "gf_math.h"

/**
 * Multiply an 8-bit polynomial by x (represented as 0x02).
 *
 * If the highest bit (bit 7) is 0, multiplying by x is simply a 1-bit left shift:
 *   (b << 1)
 * If the highest bit is 1, shifting left produces an x^8 term, which overflows
 * our 8-bit field. In GF(2^8), we reduce modulo m(x) = x^8 + x^4 + x^3 + x + 1.
 * Since the x^8 bit falls off the 8-bit register, subtracting m(x) (which is XOR)
 * means XORing the remaining lower 8 bits with (0x11B & 0xFF) = 0x1B.
 *
 * Hardware implementation:
 *   wire [7:0] shifted = {b[6:0], 1'b0};
 *   assign out = b[7] ? (shifted ^ 8'h1b) : shifted;
 */
uint8_t gf_xtime(uint8_t b)
{
    /* Check if the most significant bit (b7) is set */
    uint8_t high_bit_set = (b & 0x80) != 0;

    /* Shift left by 1 bit (equivalent to multiplying by x) */
    uint8_t shifted = (uint8_t)(b << 1);

    /* If bit 7 was 1, XOR with the reduction polynomial constant 0x1B */
    if (high_bit_set) {
        return shifted ^ 0x1B;
    } else {
        return shifted;
    }
}

/**
 * Multiply two bytes 'a' and 'b' in GF(2^8) bit-by-bit using Russian Peasant
 * multiplication (shifts and conditional XORs).
 *
 * For each of the 8 bits in 'b' (from bit 0 to bit 7):
 *   - If the current bit of 'b' is 1, XOR 'a' into the running accumulator.
 *   - Advance 'a' to the next power of x using gf_xtime(a).
 *   - Shift 'b' right by 1 to inspect the next bit.
 *
 * Hardware implementation:
 * This loop can be completely unrolled into an 8-level combinational circuit
 * (a tree of xtime modules and XOR gates). In Verilog, it can be written as
 * pure combinational logic with zero clock cycles.
 */
uint8_t gf_mul(uint8_t a, uint8_t b)
{
    uint8_t product = 0x00;
    uint8_t temp_a = a;
    uint8_t temp_b = b;

    for (int bit_index = 0; bit_index < 8; bit_index++) {
        /* If the lowest bit of temp_b is 1, add temp_a into product */
        if (temp_b & 0x01) {
            product ^= temp_a;  /* Addition in GF(2^8) is XOR */
        }

        /* Multiply temp_a by x for the next bit position */
        temp_a = gf_xtime(temp_a);

        /* Shift temp_b right to inspect the next bit */
        temp_b >>= 1;
    }

    return product;
}
