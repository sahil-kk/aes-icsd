/**
 * ============================================================================
 * MODULE: aes128_core.c / aes128_core.h (AES-128 Top-Level Sequencing & FSM)
 * ============================================================================
 *
 * (a) WHAT THIS STAGE DOES IN PLAIN ENGLISH:
 *     This is the "conductor" or orchestrator of the AES engine. It takes the
 *     16 bytes of plaintext and loads them into a 4x4 working table called the
 *     "state". It then sequences the state through 10 rounds of cryptographic
 *     transformations:
 *
 *       1. Pre-Round (Round 0):
 *          - AddRoundKey (with Key 0)
 *
 *       2. Main Rounds (Rounds 1 through 9):
 *          - SubBytes
 *          - ShiftRows
 *          - MixColumns
 *          - AddRoundKey (with Key r)
 *
 *       3. Final Round (Round 10):
 *          - SubBytes
 *          - ShiftRows
 *          - [Notice: MixColumns is SKIPPED here!]
 *          - AddRoundKey (with Key 10)
 *
 *     Finally, it copies the 4x4 state back out into the 16-byte ciphertext array.
 *
 * (b) WHY AES NEEDS THIS STEP:
 *     A single round of SubBytes, ShiftRows, and MixColumns is not enough to
 *     provide security. Cryptanalysis has shown that fewer than 8 rounds can be
 *     broken mathematically. Ten rounds provide a robust security margin,
 *     ensuring that the ciphertext exhibits complete statistical randomness.
 *     The final round omits MixColumns so that the encryption and decryption
 *     circuits can be structurally symmetrical.
 *
 * (c) HARDWARE PERSPECTIVE (BRIDGING TO VERILOG):
 *     In hardware, this outer loop becomes a counter register plus control logic
 *     that decides which stage runs on which clock cycle.
 *
 *     - CONCEPT: FINITE STATE MACHINE (FSM) & DATAPATH
 *       In digital system architecture, complex systems are divided into two parts:
 *       1. THE DATAPATH: The collection of arithmetic and logic units (XOR gates,
 *          S-box ROMs, MixColumns multipliers, and state registers) that physically
 *          hold and manipulate the data.
 *       2. THE CONTROLLER (FSM): A small sequential state machine that acts like
 *          the traffic cop. It tracks the current round number in a 4-bit register
 *          and outputs control signals (like 'enable_mix_columns', 'round_key_select',
 *          'done_signal') on each clock tick.
 *
 *     - WHY WE DON'T FOLD ROUND 10 INTO THE LOOP:
 *       In software, writing an 'if (round == 10)' inside a single loop might seem
 *       clever, but in hardware design, writing separate, explicit states for
 *       ROUNDS_1_TO_9 and FINAL_ROUND makes the FSM states, transition logic, and
 *       datapath multiplexers crystal clear and bug-free.
 *
 * ============================================================================
 */

#include <stdio.h>
#include "aes128_core.h"
#include "key_expansion.h"
#include "sub_bytes.h"
#include "shift_rows.h"
#include "mix_columns.h"
#include "add_round_key.h"

/**
 * Print the 4x4 state matrix in FIPS-197 format (column by column).
 */
void print_state_hex(const char *label, const uint8_t state[4][4])
{
    printf("%-8s ", label);
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            printf("%02x", state[row][col]);
        }
    }
    printf("\n");
}

/**
 * Core AES-128 encryption routine with optional step-by-step trace printing.
 */
void aes128_encrypt_verbose(const uint8_t plaintext[16],
                            const uint8_t key[16],
                            uint8_t ciphertext[16],
                            int print_trace)
{
    /*
     * HARDWARE REGISTERS:
     * - 'state' corresponds to a 128-bit register (16 flip-flops x 8 bits).
     * - 'round_keys' corresponds to the 11 stored round keys.
     * No heap allocation (malloc) is used anywhere!
     */
    uint8_t state[4][4];
    uint8_t round_keys[11][4][4];

    /* ========================================================================
     * STAGE 1: KEY EXPANSION
     * Expand the 16-byte key into 11 round keys (Round 0 through Round 10).
     * In hardware, this can be precomputed or computed on-the-fly each round.
     * ======================================================================== */
    aes128_key_expansion(key, round_keys);

    /* ========================================================================
     * STAGE 2: LOAD PLAINTEXT INTO 4x4 STATE MATRIX
     * In FIPS-197, input bytes are loaded into the state in COLUMN-MAJOR order:
     *   col 0 = plaintext[0..3]
     *   col 1 = plaintext[4..7]
     *   col 2 = plaintext[8..11]
     *   col 3 = plaintext[12..15]
     * In hardware, this is simply loading the 128 input wires into the state register.
     * ======================================================================== */
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            state[row][col] = plaintext[row + (4 * col)];
        }
    }

    if (print_trace) {
        printf("------------------------------------------------------------------\n");
        printf("ROUND 0 (Initial AddRoundKey):\n");
        print_state_hex("input", state);
        print_state_hex("k_sch", round_keys[0]);
    }

    /* ========================================================================
     * STAGE 3: PRE-ROUND (ROUND 0)
     * Initial AddRoundKey using round_keys[0] (the master key).
     * ======================================================================== */
    add_round_key(state, round_keys[0]);

    /* ========================================================================
     * STAGE 4: MAIN ROUNDS (ROUNDS 1 THROUGH 9)
     * In hardware, an FSM increments a 4-bit round counter register (1 to 9).
     * On each clock cycle, the state updates through all 4 operations.
     * ======================================================================== */
    for (int round_num = 1; round_num <= 9; round_num++) {
        if (print_trace) {
            printf("\nROUND %d:\n", round_num);
            print_state_hex("start", state);
        }

        /* 1. SubBytes (Non-linear byte substitution) */
        sub_bytes(state);
        if (print_trace) {
            print_state_hex("s_box", state);
        }

        /* 2. ShiftRows (Cyclic row rotation) */
        shift_rows(state);
        if (print_trace) {
            print_state_hex("s_row", state);
        }

        /* 3. MixColumns (Galois Field column diffusion) */
        mix_columns(state);
        if (print_trace) {
            print_state_hex("m_col", state);
        }

        /* 4. AddRoundKey (XOR with current round's key) */
        if (print_trace) {
            print_state_hex("k_sch", round_keys[round_num]);
        }
        add_round_key(state, round_keys[round_num]);
    }

    /* ========================================================================
     * STAGE 5: FINAL ROUND (ROUND 10)
     * Notice: MixColumns is deliberately OMITTED in this final round!
     * ======================================================================== */
    if (print_trace) {
        printf("\nROUND 10 (Final Round - MixColumns omitted):\n");
        print_state_hex("start", state);
    }

    /* 1. SubBytes */
    sub_bytes(state);
    if (print_trace) {
        print_state_hex("s_box", state);
    }

    /* 2. ShiftRows */
    shift_rows(state);
    if (print_trace) {
        print_state_hex("s_row", state);
    }

    /* 3. AddRoundKey (Round 10 Key) */
    if (print_trace) {
        print_state_hex("k_sch", round_keys[10]);
    }
    add_round_key(state, round_keys[10]);

    if (print_trace) {
        print_state_hex("output", state);
        printf("------------------------------------------------------------------\n");
    }

    /* ========================================================================
     * STAGE 6: UNLOAD 4x4 STATE MATRIX INTO CIPHERTEXT OUTPUT ARRAY
     * Unload column-by-column back into the 16-byte output buffer.
     * ======================================================================== */
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            ciphertext[row + (4 * col)] = state[row][col];
        }
    }
}

/**
 * Standard AES-128 encrypt entry point (silent).
 */
void aes128_encrypt(const uint8_t plaintext[16],
                    const uint8_t key[16],
                    uint8_t ciphertext[16])
{
    aes128_encrypt_verbose(plaintext, key, ciphertext, 0);
}
