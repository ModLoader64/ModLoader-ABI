#pragma once

#include "instruction.h"

namespace N64 {

class Disassembler {
public:
    Disassembler() = default;
    Disassembler(u32 instruction, u32 instruction_address) : word(instruction), address(instruction_address) {
    }

    void Decode(u32 instruction, u32 instruction_address) {
        word = instruction;
        address = instruction_address;
    }
    
    Instruction_Info Analyze() const;
    
    size_t Format(char* output, size_t capacity) const; // unknown instructions emit .word

private:
    u32 Branch_Target() const;
    u32 Jump_Target() const;

    u32 word = 0;
    u32 address = 0;
};

}
