/**
 * ============================================================================
 * MODULE: main.c (AES-128 Hardware Verification Testbench & Teaching Harness)
 * ============================================================================
 *
 * PURPOSE:
 * This program serves two primary purposes:
 *
 * 1. An automated verification suite: It executes standard NIST / FIPS-197
 *    test vectors and checks the computed ciphertext against known reference
 *    outputs, printing PASS or FAIL.
 *
 * 2. An educational trace generator: For the primary FIPS-197 Appendix B test
 *    vector, it outputs the complete internal state after EVERY single
 *    transformation (start, SubBytes, ShiftRows, MixColumns, AddRoundKey)
 *    across all 10 rounds.
 *
 * HOW TO USE THIS WHEN WRITING VERILOG LATER:
 * When you write your AES Verilog testbench, you can log your Verilog internal
 * registers at every clock cycle to a file or simulator console. You can then
 * directly compare your Verilog output side-by-side with this C trace to locate
 * the exact round, stage, or byte where a bug occurs!
 *
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include "aes128_core.h"

/* Structure to hold a test vector */
typedef struct {
    const char *name;
    uint8_t key[16];
    uint8_t plaintext[16];
    uint8_t expected_ciphertext[16];
} aes128_test_vector_t;

/**
 * Helper to print a 16-byte buffer in hexadecimal notation.
 */
static void print_bytes_hex(const uint8_t *bytes, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        printf("%02x", bytes[i]);
    }
    printf("\n");
}

int main(void)
{
    printf("====================================================================\n");
    printf("     AES-128 C REFERENCE IMPLEMENTATION FOR HARDWARE STUDENTS       \n");
    printf("====================================================================\n\n");

    /*
     * TEST VECTOR 1: NIST FIPS-197 Appendix B
     * This is the canonical test vector documented in the AES standard with full
     * intermediate round values.
     */
    aes128_test_vector_t test_vectors[] = {
        {
            .name = "FIPS-197 Appendix B (Standard Trace Vector)",
            .key = {
                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
            },
            .plaintext = {
                0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
            },
            .expected_ciphertext = {
                0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30,
                0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a
            }
        },
        /*
         * TEST VECTOR 2: NIST SP 800-38A (Block 1)
         * Key: 2b7e151628aed2a6abf7158809cf4f3c
         * Plaintext: 6bc1bee22e409f96e93d7e117393172a
         */
        {
            .name = "NIST SP 800-38A (ECB Example Vector 1)",
            .key = {
                0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
            },
            .plaintext = {
                0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
                0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a
            },
            .expected_ciphertext = {
                0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60,
                0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97
            }
        },
        /*
         * TEST VECTOR 3: NIST SP 800-38A (ECB Block 2)
         * Key: 2b7e151628aed2a6abf7158809cf4f3c
         * Plaintext: ae2d8a571e03ac9c9eb76fac45af8e51
         */
        {
            .name = "NIST SP 800-38A (ECB Block 2)",
            .key = {
                0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
            },
            .plaintext = {
                0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
                0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51
            },
            .expected_ciphertext = {
                0xf5, 0xd3, 0xd5, 0x85, 0x03, 0xb9, 0x69, 0x9d,
                0xe7, 0x85, 0x89, 0x5a, 0x96, 0xfd, 0xba, 0xaf
            }
        },
        /*
         * TEST VECTOR 4: FIPS-197 Appendix B (Cipher Example)
         * Key: 2b7e151628aed2a6abf7158809cf4f3c
         * Plaintext: 3243f6a8885a308d313198a2e0370734
         */
        {
            .name = "FIPS-197 Appendix B (Cipher Example)",
            .key = {
                0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
            },
            .plaintext = {
                0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
                0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34
            },
            .expected_ciphertext = {
                0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb,
                0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32
            }
        }
    };

    const size_t num_vectors = sizeof(test_vectors) / sizeof(test_vectors[0]);
    int all_passed = 1;

    /* ------------------------------------------------------------------------
     * PART 1: DETAILED STEP-BY-STEP TRACE (VECTOR 1)
     * ------------------------------------------------------------------------ */
    printf(">>> PART 1: FIPS-197 APPENDIX B DETAILED STEP-BY-STEP TRACE <<<\n");
    printf("Key        : ");
    print_bytes_hex(test_vectors[0].key, 16);
    printf("Plaintext  : ");
    print_bytes_hex(test_vectors[0].plaintext, 16);
    printf("\nIntermediate round transformations (compare against FIPS-197 App. B):\n\n");

    uint8_t actual_ciphertext[16];
    aes128_encrypt_verbose(test_vectors[0].plaintext,
                           test_vectors[0].key,
                           actual_ciphertext,
                           1 /* Enable trace */);

    printf("\nComputed Ciphertext : ");
    print_bytes_hex(actual_ciphertext, 16);
    printf("Expected Ciphertext : ");
    print_bytes_hex(test_vectors[0].expected_ciphertext, 16);

    if (memcmp(actual_ciphertext, test_vectors[0].expected_ciphertext, 16) == 0) {
        printf("[RESULT] Vector 1 PASS\n\n");
    } else {
        printf("[RESULT] Vector 1 FAIL\n\n");
        all_passed = 0;
    }

    /* ------------------------------------------------------------------------
     * PART 2: ADDITIONAL VERIFICATION SUITE
     * ------------------------------------------------------------------------ */
    printf("====================================================================\n");
    printf(">>> PART 2: ADDITIONAL NIST TEST VECTORS <<<\n");
    printf("====================================================================\n");

    for (size_t i = 1; i < num_vectors; i++) {
        printf("\nTest %zu: %s\n", i + 1, test_vectors[i].name);
        printf("Key       : ");
        print_bytes_hex(test_vectors[i].key, 16);
        printf("Plaintext : ");
        print_bytes_hex(test_vectors[i].plaintext, 16);

        aes128_encrypt(test_vectors[i].plaintext,
                       test_vectors[i].key,
                       actual_ciphertext);

        printf("Computed  : ");
        print_bytes_hex(actual_ciphertext, 16);
        printf("Expected  : ");
        print_bytes_hex(test_vectors[i].expected_ciphertext, 16);

        if (memcmp(actual_ciphertext, test_vectors[i].expected_ciphertext, 16) == 0) {
            printf("[RESULT] PASS\n");
        } else {
            printf("[RESULT] FAIL\n");
            all_passed = 0;
        }
    }

    printf("\n====================================================================\n");
    if (all_passed) {
        printf(">>> ALL %zu AES-128 TEST VECTORS PASSED SUCCESSFULLY! <<<\n", num_vectors);
    } else {
        printf(">>> VERIFICATION FAILED! <<<\n");
    }
    printf("====================================================================\n");

    return all_passed ? 0 : 1;
}
