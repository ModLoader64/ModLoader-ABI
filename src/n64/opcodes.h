#pragma once

#include "instruction.h"

namespace N64 {

enum Operand_Format : u8 {
    Format_None,
    Format_D_S_T,
    Format_D_T_S,
    Format_D_T_Shift,
    Format_S_T,
    Format_D,
    Format_S,
    Format_Jalr, // [rd,] rs
    Format_T_S_Signed,
    Format_T_S_Unsigned,
    Format_T_Upper,
    Format_T_Memory, // rt, offset(base)
    Format_Ft_Memory, // ft, offset(base)
    Format_Cache, // op, offset(base)
    Format_S_T_Branch,
    Format_S_Branch,
    Format_Branch,
    Format_Jump,
    Format_Code, // [code]
    Format_T_Cop0,
    Format_T_Fs,
    Format_T_Fcr,
    Format_Fd_Fs_Ft,
    Format_Fd_Fs,
    Format_Fs_Ft,
    Format_S_Immediate,
};

struct Opcode_Entry {
    const char* mnemonic;
    u32 match;
    u32 mask;
    Operand_Format format;
};

constexpr u32 Special(Special_Function function) {
    return static_cast<u32>(function);
}

constexpr u32 Op(Opcode opcode) {
    return static_cast<u32>(opcode) << 26;
}

constexpr u32 Regimm(Regimm_Function function) {
    return Op(Opcode::Regimm) | (static_cast<u32>(function) << 16);
}

constexpr u32 Cop0(Coprocessor_Operation operation) {
    return Op(Opcode::Cop0) | (static_cast<u32>(operation) << 21);
}

constexpr u32 Cop1(Coprocessor_Operation operation) {
    return Op(Opcode::Cop1) | (static_cast<u32>(operation) << 21);
}

bool Names_Equal(const char* name, u32 length, const char* candidate);
const Opcode_Entry* Find_Mnemonic(const char* name, u32 length);
const Opcode_Entry* Find_Instruction(u32 instruction);
extern const char* const gGprNames[32];
extern const char* const gCop0Names[32];

} // namespace N64
