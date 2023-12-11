# Folder where the source files are located
SRC_DIR := .

# Collect all .c files in the source directory
C_SRC := $(wildcard $(SRC_DIR)/*.c)

# Collect all .h files in the source directory
INC := $(wildcard $(SRC_DIR)/*.h)

# Compiler and flags
CC := gcc
CFLAGS := -DRISCV

# Executable name
TARGET := script_disparity

# Default target
all: $(TARGET)

# Linking rule
$(TARGET): $(C_SRC)
	riscv64-unknown-elf-gcc $(CFLAGS) -o $@ $^ -lm

# Clean rule
clean:
	rm -f $(TARGET)

.PHONY: all clean

