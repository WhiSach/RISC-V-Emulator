#include <vector>
#include <cstdint>
#include <iostream>

struct DRAM {
    uint64_t size = 1024 * 1024 * 128;
    std::vector<uint8_t> dram;

    // Initialize the memory vector to the defined size (128MB)
    DRAM() {
        dram.resize(size);
    }

    // Read 'nbytes' from memory at 'addr'.
    // Combines bytes using little-endian ordering (LSB at lowest address).
    uint64_t load(uint64_t addr, uint64_t nbytes) {
        uint64_t value = 0;
        for (uint64_t i = 0; i < nbytes; i++) {
            value |= (uint64_t)dram[addr + i] << (i * 8);
        }
        return value;
    }

    // Write 'nbytes' of 'value' to memory at 'addr'.
    // Splits the value into bytes, storing LSB first (Little Endian).
    void store(uint64_t addr, uint64_t nbytes, uint64_t value) {
        for (uint64_t i = 0; i < nbytes; i++) {
            dram[addr + i] = (value >> (i * 8)) & 0xFF;
        }
    }

    uint64_t load8(uint64_t addr) { return load(addr, 1); }
    uint64_t load16(uint64_t addr) { return load(addr, 2); }
    uint64_t load32(uint64_t addr) { return load(addr, 4); }
    uint64_t load64(uint64_t addr) { return load(addr, 8); }

    void store8(uint64_t addr, uint64_t value) { store(addr, 1, value); }
    void store16(uint64_t addr, uint64_t value) { store(addr, 2, value); }
    void store32(uint64_t addr, uint64_t value) { store(addr, 4, value); }
    void store64(uint64_t addr, uint64_t value) { store(addr, 8, value); }
} DRAM;

struct Bus {
    struct DRAM dram;
} Bus;


struct CPU {
    uint64_t registers[32];
    uint64_t pc;
    std::vector<uint8_t> code;
    struct Bus bus;

    uint32_t fetch() {
        CPU *cpu = (CPU*)this;
        uint32_t index = cpu->pc / 4;
        return ((uint32_t)cpu->code[index * 4 + 0]) | ((uint32_t)cpu->code[index * 4 + 1] << 8) | ((uint32_t)cpu->code[index * 4 + 2] << 16) | (uint32_t)cpu->code[index * 4 + 3] << 24;
    }
    
    uint32_t execute(uint32_t instruction) {
        uint32_t opcode = instruction & 0x7F;
        uint32_t rd = (instruction >> 7) & 0x1F;
        uint32_t rs1 = (instruction >> 15) & 0x1F;
        uint32_t rs2 = (instruction >> 20) & 0x1F;
        // uint32_t funct3 = (instruction >> 12) & 0x7;
        // uint32_t funct7 = (instruction >> 25) & 0x7F;
        uint32_t imm = (instruction >> 20) & 0xFFF;
        // uint32_t shamt = (instruction >> 20) & 0x1F;
        // uint32_t immU = (instruction >> 12) & 0xFFFFF;

        switch (opcode) {
            case 0x13: // ADDI
                this->registers[rd] = this->registers[rs1] + ((instruction >> 20) & 0xFFF);
                break;
            case 0x33: // ADD
                this->registers[rd] = this->registers[rs1] + this->registers[rs2];
                break;
            default:
                break;
        }
        return 0;
    }

} CPU;

int main() {
    // Load a simple test program:
    // 1. ADDI x1, x0, 5  (Opcode: 0x13, rd: 1, imm: 5) -> Hex: 00500093
    // 2. ADD  x2, x1, x1 (Opcode: 0x33, rd: 2, rs1: 1, rs2: 1) -> Hex: 00108133
    CPU.code = {
        0x93, 0x00, 0x50, 0x00, 
        0x33, 0x81, 0x10, 0x00
    };

    while (CPU.pc < CPU.code.size())  {
        uint32_t instruction = CPU.fetch();
        CPU.execute(instruction);
        CPU.pc += 4;
    }

    std::cout << "x1: " << CPU.registers[1] << " (Expected: 5)" << std::endl;
    std::cout << "x2: " << CPU.registers[2] << " (Expected: 10)" << std::endl;
};