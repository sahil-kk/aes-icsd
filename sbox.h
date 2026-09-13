#ifndef SBOX_H
#define SBOX_H

#include <stdint.h>

/**
 * Look up a single byte in the AES forward Substitution Box (S-Box).
 *
 * @param in  The 8-bit input byte to substitute.
 * @return    The 8-bit substituted output byte.
 *
 * Hardware perspective:
 * This function models an 8-bit input, 8-bit output Read-Only Memory (ROM)
 * or Look-Up Table (LUT). In hardware, this is purely combinational: the input
 * byte acts as an 8-bit address wire bus, and the stored byte appears on the
 * 8-bit output data bus after a brief propagation delay.
 */
uint8_t sbox_lookup(uint8_t in);

/**
 * Look up a single byte in the AES Inverse Substitution Box (Inv S-Box).
 * Used during AES decryption.
 *
 * @param in  The 8-bit input byte to reverse-substitute.
 * @return    The 8-bit original byte.
 */
uint8_t inv_sbox_lookup(uint8_t in);

#endif /* SBOX_H */
