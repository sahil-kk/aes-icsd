# AES-128 C Reference Implementation & Hardware Bridge

> **A Teaching Tool for First-Year Electronics & Computer Engineering Students**
>
> Welcome! This codebase is an educational C99 reference implementation of the **AES-128 (Advanced Encryption Standard)** encryption algorithm.
> It has been designed specifically for students who are comfortable with C programming, have **no prior cryptography experience**, and are preparing to translate AES into **Verilog HDL** for digital hardware implementation (FPGA or ASIC).

---

## Table of Contents
1. [AES in One Page (Conceptual Overview)](#1-aes-in-one-page-conceptual-overview)
2. [Why No `malloc` and No Global Variables?](#2-why-no-malloc-and-no-global-variables)
3. [Glossary of Digital Hardware Terms](#3-glossary-of-digital-hardware-terms)
4. [File Structure & Stage Breakdown](#4-file-structure--stage-breakdown)
5. [C-to-Verilog Module Mapping & Port Table](#5-c-to-verilog-module-mapping--port-table)
6. [Building and Running](#6-building-and-running)
7. [FIPS-197 Trace Verification](#7-fips-197-trace-verification)
8. [What's Next: Your Verilog Translation Roadmap](#8-whats-next-your-verilog-translation-roadmap)

---

## 1. AES in One Page (Conceptual Overview)

### What is AES?
AES is a **symmetric block cipher**.
- **Symmetric**: The sender and receiver use the exact same secret key.
- **Block Cipher**: Data is processed in fixed chunks of **128 bits (16 bytes)** at a time, known as a **block**.
- **AES-128**: The secret key is **128 bits (16 bytes)** long.

### The 4x4 State Matrix
Inside AES, the 16 bytes of data being encrypted are visualized as a 4-row by 4-column grid called the **State**, arranged in **column-major order**:

```
Input byte array: [ b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15 ]

Arranged as 4x4 State Matrix:
+------+------+------+------+
|  b0  |  b4  |  b8  |  b12 |  <- Row 0
+------+------+------+------+
|  b1  |  b5  |  b9  |  b13 |  <- Row 1
+------+------+------+------+
|  b2  |  b6  |  b10 |  b14 |  <- Row 2
+------+------+------+------+
|  b3  |  b7  |  b11 |  b15 |  <- Row 3
+------+------+------+------+
  Col 0  Col 1  Col 2  Col 3
```

### Why Does AES Have 10 Rounds?
To make encryption unbreakable, a cipher must achieve two fundamental properties:
1. **Confusion**: Obscuring the relationship between the key and the ciphertext (handled by `SubBytes`).
2. **Diffusion**: Spreading the influence of each input bit across the entire output (handled by `ShiftRows` and `MixColumns`).

A single round is not secure; attackers could solve it with math. However, repeating these steps across **10 rounds** causes an "avalanche effect": changing a single bit in the input plaintext scrambles more than 50% of the output bits by Round 2, and renders the result completely indistinguishable from random noise by Round 10.

### The Round Sequence
Every round performs four operations, with two special exceptions:
```
           Plaintext (16 bytes)
                    │
                    ▼
          [ AddRoundKey (Round 0) ]    <-- Initial Key Blending
                    │
   ┌────────────────┴────────────────┐
   │  FOR ROUNDS 1 TO 9:             │
   │    1. SubBytes    (Confusion)   │
   │    2. ShiftRows   (Diffusion)   │
   │    3. MixColumns  (Diffusion)   │
   │    4. AddRoundKey (Key Injection│
   └────────────────┬────────────────┘
                    │
   ┌────────────────┴────────────────┐
   │  ROUND 10 (Final Round):        │
   │    1. SubBytes                  │
   │    2. ShiftRows                 │
   │    [ MixColumns is OMITTED! ]   │
   │    3. AddRoundKey (Round 10)    │
   └────────────────┬────────────────┘
                    │
                    ▼
          Ciphertext (16 bytes)
```

### Why Does Key Expansion Exist?
If AES used the exact same 16-byte key in every round, an attacker could exploit structural symmetry. Key Expansion stretches your 16-byte master key into **11 distinct 16-byte Round Keys** (176 bytes total), one for Round 0 through Round 10.

---

## 2. Why No `malloc` and No Global Variables?

If you inspect the C code, you will notice two strict rules:

1. **No `malloc` or `free` (Dynamic Memory)**:
   - *Why in C?* We only use fixed-size stack arrays (e.g. `uint8_t state[4][4]`).
   - *Why for Hardware?* **Physical silicon chips have no heap.** In hardware, wires and memory cells (flip-flops) must physically exist on the silicon die before power is turned on. You cannot "allocate" new silicon while the chip is running!
2. **No Global Variables**:
   - *Why in C?* Every function receives input pointers and writes output pointers.
   - *Why for Hardware?* Each function is written to mirror a **hardware module**. Input parameters are the module's **input ports (wires)**, and output pointers are its **output ports (wires)**. Global state creates invisible dependencies that have no direct physical equivalent in a clean hardware schematic.

---

## 3. Glossary of Digital Hardware Terms

When transitioning from software to digital logic, keep this glossary handy:

| Hardware Term | Plain-Language Definition |
|:---|:---|
| **Combinational Logic** | Logic circuits whose output depends *only* on the current inputs right now (e.g., XOR gates, AND gates, wire shifts). If inputs change, outputs change after a tiny propagation delay. No clock needed. |
| **Sequential Logic** | Logic circuits whose output depends on *past stored values* as well as current inputs. They update only on clock edges. |
| **Register / Flip-Flop** | A physical electronic memory cell that captures and holds binary data (e.g., 8 bits or 128 bits) on the rising edge of a clock signal. |
| **Clock Cycle** | One periodic pulse of a square-wave clock signal ($0 \to 1 \to 0$). In synchronous hardware, all major operations step forward by one beat per clock cycle. |
| **Finite State Machine (FSM)** | A digital controller circuit with a memory register (storing the current "state", like `ROUND_1`, `ROUND_2`, etc.) and combinational logic that decides what to do and which state to jump to next. |
| **ROM / LUT (Look-Up Table)**| Read-Only Memory. Instead of calculating a complex mathematical function with transistors, the circuit stores precalculated answers at specific address wires. |
| **Propagation Delay** | The tiny physical time (measured in picoseconds or nanoseconds) it takes for an electrical voltage to travel through transistors and wires. |
| **Interconnect / Wire Routing** | Physical metal traces connecting one logic gate to another. Operations like shifting wires left or right have **zero delay and zero gates** in hardware. |
| **Hardware Trade-off (Area vs. Speed)**| In software, optimization usually means fewer CPU instructions. In hardware, you can make a circuit 16x faster by copying it 16 times on silicon, but it consumes 16x more chip area and power. |

---

## 4. File Structure & Stage Breakdown

Each stage of AES is isolated in its own `.c`/`.h` file pair so you can read, test, and translate them one by one:

```
.
├── Makefile            # Standalone build system (C99, -Wall -Wextra -pedantic)
├── README.md           # This educational guide
├── sbox.h / sbox.c     # Stage 1: 256-byte S-box and Inverse S-box ROM lookup
├── gf_math.h / gf_math.c # Stage 2: Galois Field GF(2^8) arithmetic (xtime, mul)
├── rcon.h              # Stage 3a: Round Constant lookup table
├── key_expansion.h / .c# Stage 3b: Key Schedule (generates 11 round keys)
├── add_round_key.h / .c# Stage 4: 16-byte parallel XOR gate stage
├── sub_bytes.h / .c    # Stage 5: State S-box byte substitution
├── shift_rows.h / .c   # Stage 6: Row permutation (free wire routing)
├── mix_columns.h / .c  # Stage 7: Column matrix multiplication in GF(2^8)
├── aes128_core.h / .c  # Stage 8: Top-level sequencer & FSM controller
└── main.c              # Stage 9: Verification testbench & FIPS-197 trace generator
```

---

## 5. C-to-Verilog Module Mapping & Port Table

When you write your Verilog project, create one Verilog module (`.v` file) for each `.c` file below:

| C Source File | Target Verilog Module | Logic Type | Hardware Port Interface (Verilog) | Hardware Implementation Notes |
|:---|:---|:---:|:---|:---|
| `sbox.c` | `sbox.v` | Combinational | `input [7:0] in`,<br>`output [7:0] out` | Synthesizes into an 8-bit ROM or FPGA block RAM (BRAM). |
| `gf_math.c` | `gf_xtime.v` | Combinational | `input [7:0] in`,<br>`output [7:0] out` | 1-bit wire shift + conditional XOR with `8'h1b` (`assign out = in[7] ? {in[6:0], 1'b0} ^ 8'h1b : {in[6:0], 1'b0};`). |
| `add_round_key.c` | `add_round_key.v` | Combinational | `input [127:0] state_in`,<br>`input [127:0] round_key`,<br>`output [127:0] state_out` | 128 parallel XOR gates (`assign state_out = state_in ^ round_key;`). Minimal delay. |
| `shift_rows.c` | `shift_rows.v` | Pure Wiring (0 gates!) | `input [127:0] state_in`,<br>`output [127:0] state_out` | Zero transistors! Just rewire byte lanes: row 0 untouched, row 1 rotated by 1 byte, row 2 by 2 bytes, row 3 by 3 bytes. |
| `mix_columns.c` | `mix_columns.v` | Combinational | `input [127:0] state_in`,<br>`output [127:0] state_out` | 4 parallel column mixers. Each column uses XOR trees and `gf_xtime` instances. |
| `sub_bytes.c` | `sub_bytes.v` | Combinational (or Sequential) | `input [127:0] state_in`,<br>`output [127:0] state_out` | **Design choice**: Instantiate 16 parallel `sbox` modules for 1-cycle throughput, or 1 `sbox` module shared across 16 clock cycles to save FPGA area. |
| `key_expansion.c` | `key_expansion.v` | Sequential | `input clk`, `input rst_n`,<br>`input start`,<br>`input [127:0] key`,<br>`output [1407:0] all_keys`,<br>`output ready` | Calculates one round key per clock cycle using registers, `rot_word`, 4 `sbox` instances, and `rcon`. |
| `aes128_core.c` | `aes128_top.v` | Sequential (FSM) | `input clk`, `input rst_n`,<br>`input start`,<br>`input [127:0] plaintext`,<br>`input [127:0] key`,<br>`output [127:0] ciphertext`,<br>`output done` | Top-level module: contains a 128-bit state register, round counter (0 to 10), multiplexers, and FSM state registers. |

---

## 6. Building and Running

### Prerequisites
A standard C99 compiler (`gcc`, `clang`, or MinGW on Windows) and `make`. No external libraries or crypto packages are needed.

### Commands
```bash
# Build the test executable
make

# Run the full test suite with verbose FIPS-197 intermediate trace
make run

# Clean build artifacts
make clean
```

---

## 7. FIPS-197 Trace Verification

When you execute `make run`, the harness runs **4 standard test vectors**:
1. **FIPS-197 Appendix C.1 / Table B.1**:
   - Master Key: `000102030405060708090a0b0c0d0e0f`
   - Plaintext : `00112233445566778899aabbccddeeff`
   - Ciphertext: `69c4e0d86a7b0430d8cdb78070b4c55a`
   - *This vector prints the complete step-by-step intermediate state (`start`, `s_box`, `s_row`, `m_col`, `k_sch`) after every stage of all 10 rounds.*
2. **NIST SP 800-38A ECB Vector 1**:
   - Master Key: `2b7e151628aed2a6abf7158809cf4f3c`
   - Plaintext : `6bc1bee22e409f96e93d7e117393172a`
   - Ciphertext: `3ad77bb40d7a3660a89ecaf32466ef97`
3. **NIST SP 800-38A ECB Vector 2**:
   - Master Key: `2b7e151628aed2a6abf7158809cf4f3c`
   - Plaintext : `ae2d8a571e03ac9c9eb76fac45af8e51`
   - Ciphertext: `f5d3d58503b9699de785895a96fdbaaf`
4. **FIPS-197 Appendix B (Cipher Example)**:
   - Master Key: `2b7e151628aed2a6abf7158809cf4f3c`
   - Plaintext : `3243f6a8885a308d313198a2e0370734`
   - Ciphertext: `3925841d02dc09fbdc118597196a0b32`

All 4 test vectors execute automatically and report `[RESULT] PASS`.

---

## 8. What's Next: Your Verilog Translation Roadmap

When you begin coding in Verilog, **do not start by writing the entire AES cipher at once.** That is a recipe for painful debugging. Instead, follow this recommended milestone progression:

### Step 1: Start with Pure Combinational Modules (No Clock)
- **`shift_rows.v`**: Write this first! It has zero logic gates. Simply connect 128 input bits to 128 output bits in the shifted byte order. Write a small Verilog testbench and verify it works.
- **`add_round_key.v`**: Next easiest! Connect 128 XOR gates using `assign state_out = state_in ^ round_key;`.
- **`gf_xtime.v`**: Implement the 8-bit wire shift and conditional XOR.

### Step 2: Implement the ROM Look-Up Table
- **`sbox.v`**: Implement the 256-entry table using a Verilog `case (in)` statement or a `reg [7:0] rom [0:255]` initialized via `$readmemh`.
- **`sub_bytes.v`**: Instantiate 16 copies of your `sbox.v` module in parallel.

### Step 3: Implement Column Mixing
- **`mix_columns.v`**: Instantiate `gf_xtime` and wire up the XOR formulas from `mix_single_column()` for all 4 columns.

### Step 4: Step into Sequential Logic (Add the Clock!)
- **`key_expansion.v`**: Now introduce clock and reset. Build a register to store the round keys and an FSM or counter that steps from Round 1 to 10.

### Step 5: Top-Level Integration (`aes128_top.v`)
- Build the 128-bit `state` register.
- Create an FSM controller with states: `IDLE`, `LOAD`, `ROUND_0`, `ROUNDS_1_TO_9`, `ROUND_10`, `DONE`.
- Log the 128-bit register in your Verilog testbench after every clock cycle using `$display("%h", state);`.
- **Compare your Verilog simulator output line-by-line against the C program trace output generated by `make run`!** If round 3 byte 2 differs, you can pinpoint the exact module causing the issue immediately.

Happy hardware hacking!
