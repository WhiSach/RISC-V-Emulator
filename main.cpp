#include <vector>
#include <cstdint>
#include <iostream>

// CSR Addresses
const uint16_t SIE = 0x104;
const uint16_t MIE = 0x304;
const uint16_t MIDELEG = 0x303;



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
    uint64_t DRAM_BASE = 0x80000000; 

    uint64_t load(uint64_t addr, uint64_t nbytes) {
        if (DRAM_BASE <= addr) {
            return dram.load(addr - DRAM_BASE, nbytes);
        }
        return 0;
    }
    void store(uint64_t addr, uint64_t nbytes, uint64_t value) {
        if (DRAM_BASE <= addr) {
            dram.store(addr - DRAM_BASE, nbytes, value);
        }
    }
} Bus;


struct CPU {
    uint64_t registers[32];
    uint64_t pc;
    std::vector<uint8_t> code;
    struct Bus bus;
    uint64_t csrs[4096];

    uint32_t fetch() {
        CPU *cpu = (CPU*)this;
        uint32_t index = cpu->pc / 4;
        return ((uint32_t)cpu->code[index * 4 + 0]) | ((uint32_t)cpu->code[index * 4 + 1] << 8) | ((uint32_t)cpu->code[index * 4 + 2] << 16) | (uint32_t)cpu->code[index * 4 + 3] << 24;
    }

    uint64_t load_csrs(uint64_t addr) {
        // Load CSR values into the CPU's CSR array
        switch (addr) {
            case SIE:
                // Return only the bits delegated to Supervisor mode
                return csrs[MIE] & csrs[MIDELEG];
            default:
                // Return the raw value for all other addresses
                return csrs[addr];
        }
    }

    void store_csrs(uint64_t addr, uint64_t value) {
        // Store CSR values into the CPU's CSR array
        switch (addr) {
            case SIE:
                // Update only the bits delegated to Supervisor mode
                csrs[MIE] = (csrs[MIE] & ~csrs[MIDELEG]) | (value & csrs[MIDELEG]);
                break;
            default:
                // Store the raw value for all other addresses
                csrs[addr] = value;
                break;
        }
    }

    uint32_t execute(uint32_t instruction) {
        uint32_t opcode = instruction & 0x7F;
        uint32_t rd = (instruction >> 7) & 0x1F;
        uint32_t rs1 = (instruction >> 15) & 0x1F;
        uint32_t rs2 = (instruction >> 20) & 0x1F;
        uint32_t funct3 = (instruction >> 12) & 0x7;
        // uint32_t funct7 = (instruction >> 25) & 0x7F;
        uint32_t imm = (instruction >> 20) & 0xFFF;
        // uint32_t shamt = (instruction >> 20) & 0x1F;
        // uint32_t immU = (instruction >> 12) & 0xFFFFF;

        switch (opcode) {
            case 0x03: { // LOAD
                int64_t imm_i = (int32_t)(imm << 20) >> 20;
                uint64_t addr = registers[rs1] + imm_i;
                switch (funct3) {
                    case 0x0: registers[rd] = (int64_t)(int8_t)bus.load(addr, 1); break; // LB
                    case 0x1: registers[rd] = (int64_t)(int16_t)bus.load(addr, 2); break; // LH
                    case 0x2: registers[rd] = (int64_t)(int32_t)bus.load(addr, 4); break; // LW
                    case 0x3: registers[rd] = bus.load(addr, 8); break; // LD
                    case 0x4: registers[rd] = (uint64_t)bus.load(addr, 1); break; // LBU
                    case 0x5: registers[rd] = (uint64_t)bus.load(addr, 2); break; // LHU
                    case 0x6: registers[rd] = (uint64_t)bus.load(addr, 4); break; // LWU
                }
                break;
            }
            case 0x13: // ADDI
                this->registers[rd] = this->registers[rs1] + ((int32_t)(imm << 20) >> 20);
                break;
            case 0x23: { // STORE
                uint32_t imm_s = ((instruction >> 25) << 5) | ((instruction >> 7) & 0x1F);
                int64_t imm_s_sext = (int32_t)(imm_s << 20) >> 20;
                uint64_t addr = registers[rs1] + imm_s_sext;
                switch (funct3) {
                    case 0x0: bus.store(addr, 1, registers[rs2]); break; // SB
                    case 0x1: bus.store(addr, 2, registers[rs2]); break; // SH
                    case 0x2: bus.store(addr, 4, registers[rs2]); break; // SW
                    case 0x3: bus.store(addr, 8, registers[rs2]); break; // SD
                }
                break;
            }
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
    // Initialize register x1 to DRAM_BASE
    CPU.registers[1] = 0x80000000;

    // Load a simple test program:
    // 1. ADDI x2, x0, 42  (Opcode: 0x13, rd: 2, imm: 42) -> Hex: 02A00113
    // 2. SB   x2, 0(x1)   (Opcode: 0x23, rs1: 1, rs2: 2, imm: 0) -> Hex: 00208023
    // 3. LB   x3, 0(x1)   (Opcode: 0x03, rd: 3, rs1: 1, imm: 0) -> Hex: 00008183
    CPU.code = {
        0x13, 0x01, 0xA0, 0x02, 
        0x23, 0x80, 0x20, 0x00,
        0x83, 0x81, 0x00, 0x00
    };

    while (CPU.pc < CPU.code.size())  {
        uint32_t instruction = CPU.fetch();
        CPU.execute(instruction);
        CPU.pc += 4;
    }

    std::cout << "x2: " << CPU.registers[2] << " (Expected: 42)" << std::endl;
    std::cout << "x3: " << CPU.registers[3] << " (Expected: 42)" << std::endl;
};