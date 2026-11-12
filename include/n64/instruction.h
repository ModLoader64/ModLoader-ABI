#pragma once

#include "types.h"

namespace N64 {

enum class Opcode : u32 {
    Special = 0x00,
    Regimm = 0x01,
    J = 0x02,
    Jal = 0x03,
    Beq = 0x04,
    Bne = 0x05,
    Blez = 0x06,
    Bgtz = 0x07,
    Addi = 0x08,
    Addiu = 0x09,
    Slti = 0x0A,
    Sltiu = 0x0B,
    Andi = 0x0C,
    Ori = 0x0D,
    Xori = 0x0E,
    Lui = 0x0F,
    Cop0 = 0x10,
    Cop1 = 0x11,
    Beql = 0x14,
    Bnel = 0x15,
    Blezl = 0x16,
    Bgtzl = 0x17,
    Daddi = 0x18,
    Daddiu = 0x19,
    Ldl = 0x1A,
    Ldr = 0x1B,
    Lb = 0x20,
    Lh = 0x21,
    Lwl = 0x22,
    Lw = 0x23,
    Lbu = 0x24,
    Lhu = 0x25,
    Lwr = 0x26,
    Lwu = 0x27,
    Sb = 0x28,
    Sh = 0x29,
    Swl = 0x2A,
    Sw = 0x2B,
    Sdl = 0x2C,
    Sdr = 0x2D,
    Swr = 0x2E,
    Cache = 0x2F,
    Ll = 0x30,
    Lwc1 = 0x31,
    Lld = 0x34,
    Ldc1 = 0x35,
    Ld = 0x37,
    Sc = 0x38,
    Swc1 = 0x39,
    Scd = 0x3C,
    Sdc1 = 0x3D,
    Sd = 0x3F,
};

enum class Special_Function : u32 {
    Sll = 0x00,
    Srl = 0x02,
    Sra = 0x03,
    Sllv = 0x04,
    Srlv = 0x06,
    Srav = 0x07,
    Jr = 0x08,
    Jalr = 0x09,
    Syscall = 0x0C,
    Break = 0x0D,
    Sync = 0x0F,
    Mfhi = 0x10,
    Mthi = 0x11,
    Mflo = 0x12,
    Mtlo = 0x13,
    Dsllv = 0x14,
    Dsrlv = 0x16,
    Dsrav = 0x17,
    Mult = 0x18,
    Multu = 0x19,
    Div = 0x1A,
    Divu = 0x1B,
    Dmult = 0x1C,
    Dmultu = 0x1D,
    Ddiv = 0x1E,
    Ddivu = 0x1F,
    Add = 0x20,
    Addu = 0x21,
    Sub = 0x22,
    Subu = 0x23,
    And = 0x24,
    Or = 0x25,
    Xor = 0x26,
    Nor = 0x27,
    Slt = 0x2A,
    Sltu = 0x2B,
    Dadd = 0x2C,
    Daddu = 0x2D,
    Dsub = 0x2E,
    Dsubu = 0x2F,
    Tge = 0x30,
    Tgeu = 0x31,
    Tlt = 0x32,
    Tltu = 0x33,
    Teq = 0x34,
    Reserved_Trap = 0x35,
    Tne = 0x36,
    Dsll = 0x38,
    Dsrl = 0x3A,
    Dsra = 0x3B,
    Dsll32 = 0x3C,
    Dsrl32 = 0x3E,
    Dsra32 = 0x3F,
};

enum class Regimm_Function : u32 {
    Bltz = 0x00,
    Bgez = 0x01,
    Bltzl = 0x02,
    Bgezl = 0x03,
    Tgei = 0x08,
    Tgeiu = 0x09,
    Tlti = 0x0A,
    Tltiu = 0x0B,
    Teqi = 0x0C,
    Tnei = 0x0E,
    Bltzal = 0x10,
    Bgezal = 0x11,
    Bltzall = 0x12,
    Bgezall = 0x13,
};

enum class Coprocessor_Operation : u32 {
    Move_From = 0x00,
    Double_Move_From = 0x01,
    Control_From = 0x02,
    Move_To = 0x04,
    Double_Move_To = 0x05,
    Control_To = 0x06,
    Branch = 0x08,
    Function = 0x10,
};

enum class Coprocessor_Branch : u32 {
    False = 0,
    True = 1,
    False_Likely = 2,
    True_Likely = 3,
};

enum class Cop0_Function : u32 {
    Tlb_Read = 0x01,
    Tlb_Write_Indexed = 0x02,
    Tlb_Write_Random = 0x06,
    Tlb_Probe = 0x08,
    Exception_Return = 0x18,
};

enum Control_Kind : u32 {
    Control_None = 0,
    Control_Branch,
    Control_Branch_Likely, // conditional; the delay slot runs only when taken
    Control_Branch_Link, // includes likely forms
    Control_Jump,
    Control_Jump_Link,
    Control_Jump_Register,
    Control_Other, // eret, syscall, break, traps: cannot be moved
};

struct Instruction_Info {
    Control_Kind control;
    u32 target; // branch or jump target, when static
    bool hasDelaySlot;
};

constexpr u32 Encode_Jump(u32 target) {
    return (static_cast<u32>(Opcode::J) << 26) | ((target >> 2) & 0x3FFFFFF);
}

constexpr u32 Encode_Jump_Link(u32 target) {
    return (static_cast<u32>(Opcode::Jal) << 26) | ((target >> 2) & 0x3FFFFFF);
}

constexpr u32 gNop = 0x00000000;

} // namespace N64
