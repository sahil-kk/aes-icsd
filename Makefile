# ==============================================================================
# Makefile for AES-128 C Educational Reference Implementation
# Target audience: First-year Electronics Engineering Students
# ==============================================================================

CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -pedantic -O2
TARGET  = aes_test

# All source files modularly broken down per AES stage
SRCS    = sbox.c \
          gf_math.c \
          key_expansion.c \
          add_round_key.c \
          sub_bytes.c \
          shift_rows.c \
          mix_columns.c \
          aes128_core.c \
          main.c

OBJS    = $(SRCS:.c=.o)
HEADERS = sbox.h \
          gf_math.h \
          rcon.h \
          key_expansion.h \
          add_round_key.h \
          sub_bytes.h \
          shift_rows.h \
          mix_columns.h \
          aes128_core.h

.PHONY: all run clean help

# Default target builds the test executable
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Convenience target to build and execute the testbench
run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

help:
	@echo "Available Makefile targets:"
	@echo "  make        - Compile the aes_test executable"
	@echo "  make run    - Compile and run all NIST test vectors with trace"
	@echo "  make clean  - Remove compiled object files and executable"
