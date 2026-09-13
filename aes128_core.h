#ifndef AES128_CORE_H
#define AES128_CORE_H

#include <stdint.h>

/**
 * Encrypt a single 16-byte (128-bit) block of plaintext using AES-128
 * with a 16-byte secret key.
 *
 * @param plaintext   Input 16-byte plaintext block.
 * @param key         Input 16-byte secret cipher key.
 * @param ciphertext  Output 16-byte encrypted ciphertext block.
 *
 * Hardware perspective:
 * This top-level function represents the top-level Verilog module (aes128_top).
 * It instantiates the round state registers, key expansion unit, round datapath
 * (SubBytes, ShiftRows, MixColumns, AddRoundKey), and a Finite State Machine (FSM)
 * controller that manages clock cycles and multiplexer select lines.
 */
void aes128_encrypt(const uint8_t plaintext[16],
                    const uint8_t key[16],
                    uint8_t ciphertext[16]);

/**
 * Verbose version of aes128_encrypt that prints intermediate states after
 * every transformation of every round, matching the format of FIPS-197
 * Appendix B.
 *
 * @param plaintext   Input 16-byte plaintext block.
 * @param key         Input 16-byte secret cipher key.
 * @param ciphertext  Output 16-byte encrypted ciphertext block.
 * @param print_trace Flag (1 to print detailed step-by-step trace, 0 to run silently).
 */
void aes128_encrypt_verbose(const uint8_t plaintext[16],
                            const uint8_t key[16],
                            uint8_t ciphertext[16],
                            int print_trace);

/**
 * Helper to print a 4x4 state matrix formatted as a single hex line,
 * column-by-column (FIPS-197 standard ordering).
 *
 * @param label  Descriptive text prefix (e.g. "start", "s_box", "k_sch").
 * @param state  4x4 matrix to display.
 */
void print_state_hex(const char *label, const uint8_t state[4][4]);

#endif /* AES128_CORE_H */
