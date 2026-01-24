# RISC-V Emulator

A lightweight, functional RISC-V emulator implemented in C++. This project simulates a 64-bit RISC-V CPU, a basic system bus, and physical memory (DRAM). It is designed to demonstrate the core mechanics of instruction fetching, decoding, and execution, including support for Control and Status Registers (CSRs).

> [!IMPORTANT]
> **Work in Progress (WIP):** This project is currently under active development. The ultimate goal is to achieve enough architectural completeness to successfully boot and support the **xv6** operating system.

## Features

* **64-bit Architecture**: Built to handle 64-bit registers and memory addressing.
* **Memory Simulation**: Includes a 128MB DRAM simulation starting at the base address `0x80000000`.
* **Instruction Set Support**:
  * **Integer Arithmetic**: `ADD`, `ADDI`.
  * **Load/Store**: Supports byte, half-word, word, and double-word operations (`LB`, `LH`, `LW`, `LD`, `LBU`, `LHU`, `LWU`, `SB`, `SH`, `SW`, `SD`).
  * **CSR Operations**: Implements atomic read/write/set/clear operations for Control and Status Registers.


* **Privilege Level Awareness**: Includes definitions and logic for Machine-mode (M-mode) and Supervisor-mode (S-mode) CSRs, such as `mstatus`, `mtvec`, `sstatus`, and `satp`.

## Project Structure

* **`DRAM`**: Manages the simulated physical memory. Handles little-endian data storage and retrieval.
* **`Bus`**: Acts as the communication layer between the CPU and peripheral devices (currently DRAM). It maps system addresses to the correct hardware component.
* **`CPU`**: The core emulation engine.
* Contains 32 general-purpose 64-bit registers and a 64-bit Program Counter (PC).
* Contains a 4096-entry CSR array.
* Implements the **Fetch-Execute** cycle.



## Control and Status Registers (CSRs)

The emulator includes specialized logic for CSR access, specifically handling delegated interrupts between Machine and Supervisor modes. Supported registers include:

* **Machine Mode**: `MHARTID`, `MSTATUS`, `MTVEC`, `MEPC`, `MCAUSE`, etc..
* **Supervisor Mode**: `SSTATUS`, `STVEC`, `SEPC`, `SCAUSE`, `SATP`, etc..

## Getting Started

### Prerequisites

* A C++ compiler supporting C++11 or higher (e.g., GCC, Clang, or MSVC).
* Visual Studio Code (optional, configurations included in `.vscode/`).

### Building and Running

1. **Compile the project**:
```bash
g++ -o risc-v-emulator main.cpp
```


2. **Run the emulator**:
```bash
./risc-v-emulator
```



## Example Usage

The `main()` function executes a hardcoded test program that demonstrates memory interaction. Below is the assembly representation of that program:

```assembly
# Initial state: x1 = 0x80000000 (DRAM_BASE)

addi x2, x0, 42    # Load decimal 42 into register x2
sb   x2, 0(x1)     # Store the byte from x2 into memory at address x1
lb   x3, 0(x1)     # Load the byte from memory at address x1 into x3

```

When run, the emulator will output the values of `x2` and `x3` to verify that the store and load operations were successful.

## Future Roadmap

* **xv6 Support**: Implement the necessary instructions and hardware traps to run the xv6 OS.
* **Instruction Set**: Expand to include Branching, Jumps, and Atomic instructions.
* **Exception Handling**: Implement full exception and interrupt handling.
* **ELF Loading**: Add support for loading compiled ELF binaries directly into simulated memory.
