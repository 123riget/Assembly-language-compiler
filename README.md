# SIMPLE Architecture: Two-Pass Assembler & Emulator 

[![Standard](https://img.shields.io/badge/Standard-ISO%20C89-blue.svg)](https://en.wikipedia.org/wiki/ANSI_C)
[![Course](https://img.shields.io/badge/Course-CS2102-orange.svg)]()
[![Architecture](https://img.shields.io/badge/Architecture-32--bit-success.svg)]()

A fully compilant, two-pass assembler and instruction set emulator for the extended SIMPLE architecture, written in strict ISO C89. This project processes custom assembly language, resolves forward references, generates binary machine code, and provides a virtual execution environment complete with state tracing. 

## System Architecture 

The simulated SIMPLE machine operates on a 32-bit architecture with a specialized register set. 

* **Program Counter (PC):** Tracks the memory address of the next instruction
* **Stack Pointer (SP):** Manages the call stack for local variables and procedure calls.
* **Internal Stack (Registers A & B) :** Register A acts as the primary accumulator, while Register B serves as a secondary register. For example, when transferring SP to A(`sp2a`), the previous value of A is pushed to B.

Instructions are 32-bit integers, logically divided into: 
* **Bits 0-7:** Opcode (identifying 1 of 18+ valid instructions).
* **Bits 8-31:** 24-bit signed 2's complement operand (for data or memory offsets).

## The Two Pass Assembler(`assembler.exe`)

To handle forward references (e.g., branching to a label defined later in the code), the assembler processes `.asm` files in two distinct passes:

* **Pass 1 (Symbol Resolution):** Scans the source code to parse labels and calculate instruction offsets without emitting any machine code. It populates an internal symbol table.
* **Pass 2 (Code Generation):** Rescans the source, substituting resolved label value into instruction operands. It generates the final binary object file (`.o`) and a formatted listing file (`.lst`).

### Error & Warning Diagnostics 
The assembler features a robust diagnostic engine capable of catching syntax and logic errors, including:
* Duplicate or invalid label definitions.
* Missing or unexpected operands (e.g., `add 5` triggers an error as `add` takes no operands).
* Unused labels (flagged as compilation warnings).

## The Emulator (`emulator.exe`)

The emulator loads the binary `.o` file into virtual memory and executes the instruction sequentially. It fully supports memory reads/writes ('ldnl', 'stnl'), stack adjustments (`adj`), arithmetic and bitwise shifts.

### Real-Time Tracing 
By passing the `-trace` flag, the emulator outputs a line-by-line snapshot of the CPU state.

**Example Trace Output:** 

## Algorithmic Implementation: Bubble Sort 

The `bubblesort.asm` file demonstrates a complete algorithmic implementation mapped to the SIMPLE instruction set.

Since the architecture lacks complex addressing modes, array traversal relies heavily on base-pointer arithmetic. The logic highlights : 
* **Memory Access:** Using `ldnl` (Load Non-Local) and `stnl` (Store Non-Local) to dynamically read and write array elements in memory based on shifting pointer offsets.
* **Element Swapping:** Utilizing the internal stack mechanics of Registers A and B to efficiently swap adjacent elements during sorting passes.
* **Loop Invariants:** Managing the inner and outer loops of the $O(n^2)$ algorithm using conditional branch instructions like `brlz` (Branch less than Zero) and `brz` (Branch Zero).

## Build Instructions 

This project adheres strictly to `gcc -std=c89` and ensures zero warnings under pedantic checks. 
```bash
# Build the Assembler
gcc -std=c89 -pedantic -W -Wall -Wpointer-arith -Wwrite-strings -Wstrict-prototypes -o assembler Project-2.c

# Build the emulator
gcc -std=c89 -pedantic -W -Wall -Wpointer-arith -Wwrite-strings -Wstrict-prototypes -o emulator Emulator2.c

```

##Testing & Execution

