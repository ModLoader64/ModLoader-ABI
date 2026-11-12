#include "opcodes.h"

#include <string.h>

namespace N64 {

namespace {

enum class Float_Format : u32 {
    Single = 16,
    Double = 17,
    Word = 20,
    Long = 21,
};

enum class Float_Function : u32 {
    Add = 0x00,
    Sub = 0x01,
    Mul = 0x02,
    Div = 0x03,
    Sqrt = 0x04,
    Abs = 0x05,
    Mov = 0x06,
    Neg = 0x07,
    Round_l = 0x08,
    Trunc_l = 0x09,
    Ceil_l = 0x0A,
    Floor_l = 0x0B,
    Round_w = 0x0C,
    Trunc_w = 0x0D,
    Ceil_w = 0x0E,
    Floor_w = 0x0F,
    Cvt_s = 0x20,
    Cvt_d = 0x21,
    Cvt_w = 0x24,
    Cvt_l = 0x25,
    Compare_F = 0x30,
    Compare_Un = 0x31,
    Compare_Eq = 0x32,
    Compare_Ueq = 0x33,
    Compare_Olt = 0x34,
    Compare_Ult = 0x35,
    Compare_Ole = 0x36,
    Compare_Ule = 0x37,
    Compare_Sf = 0x38,
    Compare_Ngle = 0x39,
    Compare_Seq = 0x3A,
    Compare_Ngl = 0x3B,
    Compare_Lt = 0x3C,
    Compare_Nge = 0x3D,
    Compare_Le = 0x3E,
    Compare_Ngt = 0x3F,
};

constexpr u32 Float(Float_Format format, Float_Function function) {
    return Op(Opcode::Cop1) | (static_cast<u32>(format) << 21) | static_cast<u32>(function);
}

constexpr u32 gSpecialMask = 0xFC00003F;
constexpr u32 gShiftMask = 0xFFE0003F; // rs must be 0
constexpr u32 gOpMask = 0xFC000000;
constexpr u32 gRegimmMask = 0xFC1F0000;
constexpr u32 gMoveMask = 0xFFE007FF;

const Opcode_Entry gOpcodes[] = {
    { "nop", 0x00000000, 0xFFFFFFFF, Format_None },
    { "sll", Special(Special_Function::Sll), gShiftMask, Format_D_T_Shift },
    { "srl", Special(Special_Function::Srl), gShiftMask, Format_D_T_Shift },
    { "sra", Special(Special_Function::Sra), gShiftMask, Format_D_T_Shift },
    { "sllv", Special(Special_Function::Sllv), gSpecialMask | 0x7C0, Format_D_T_S },
    { "srlv", Special(Special_Function::Srlv), gSpecialMask | 0x7C0, Format_D_T_S },
    { "srav", Special(Special_Function::Srav), gSpecialMask | 0x7C0, Format_D_T_S },
    { "jr", Special(Special_Function::Jr), 0xFC1FFFFF, Format_S },
    { "jalr", Special(Special_Function::Jalr), 0xFC1F07FF, Format_Jalr },
    { "syscall", Special(Special_Function::Syscall), gSpecialMask, Format_Code },
    { "break", Special(Special_Function::Break), gSpecialMask, Format_Code },
    { "sync", Special(Special_Function::Sync), 0xFFFFFFFF, Format_None },
    { "mfhi", Special(Special_Function::Mfhi), 0xFFFF07FF, Format_D },
    { "mthi", Special(Special_Function::Mthi), 0xFC1FFFFF, Format_S },
    { "mflo", Special(Special_Function::Mflo), 0xFFFF07FF, Format_D },
    { "mtlo", Special(Special_Function::Mtlo), 0xFC1FFFFF, Format_S },
    { "dsllv", Special(Special_Function::Dsllv), gSpecialMask | 0x7C0, Format_D_T_S },
    { "dsrlv", Special(Special_Function::Dsrlv), gSpecialMask | 0x7C0, Format_D_T_S },
    { "dsrav", Special(Special_Function::Dsrav), gSpecialMask | 0x7C0, Format_D_T_S },
    { "mult", Special(Special_Function::Mult), 0xFC00FFFF, Format_S_T },
    { "multu", Special(Special_Function::Multu), 0xFC00FFFF, Format_S_T },
    { "div", Special(Special_Function::Div), 0xFC00FFFF, Format_S_T },
    { "divu", Special(Special_Function::Divu), 0xFC00FFFF, Format_S_T },
    { "dmult", Special(Special_Function::Dmult), 0xFC00FFFF, Format_S_T },
    { "dmultu", Special(Special_Function::Dmultu), 0xFC00FFFF, Format_S_T },
    { "ddiv", Special(Special_Function::Ddiv), 0xFC00FFFF, Format_S_T },
    { "ddivu", Special(Special_Function::Ddivu), 0xFC00FFFF, Format_S_T },
    { "add", Special(Special_Function::Add), gSpecialMask | 0x7C0, Format_D_S_T },
    { "addu", Special(Special_Function::Addu), gSpecialMask | 0x7C0, Format_D_S_T },
    { "sub", Special(Special_Function::Sub), gSpecialMask | 0x7C0, Format_D_S_T },
    { "subu", Special(Special_Function::Subu), gSpecialMask | 0x7C0, Format_D_S_T },
    { "and", Special(Special_Function::And), gSpecialMask | 0x7C0, Format_D_S_T },
    { "or", Special(Special_Function::Or), gSpecialMask | 0x7C0, Format_D_S_T },
    { "xor", Special(Special_Function::Xor), gSpecialMask | 0x7C0, Format_D_S_T },
    { "nor", Special(Special_Function::Nor), gSpecialMask | 0x7C0, Format_D_S_T },
    { "slt", Special(Special_Function::Slt), gSpecialMask | 0x7C0, Format_D_S_T },
    { "sltu", Special(Special_Function::Sltu), gSpecialMask | 0x7C0, Format_D_S_T },
    { "dadd", Special(Special_Function::Dadd), gSpecialMask | 0x7C0, Format_D_S_T },
    { "daddu", Special(Special_Function::Daddu), gSpecialMask | 0x7C0, Format_D_S_T },
    { "dsub", Special(Special_Function::Dsub), gSpecialMask | 0x7C0, Format_D_S_T },
    { "dsubu", Special(Special_Function::Dsubu), gSpecialMask | 0x7C0, Format_D_S_T },
    { "tge", Special(Special_Function::Tge), gSpecialMask, Format_S_T },
    { "tgeu", Special(Special_Function::Tgeu), gSpecialMask, Format_S_T },
    { "tlt", Special(Special_Function::Tlt), gSpecialMask, Format_S_T },
    { "tltu", Special(Special_Function::Tltu), gSpecialMask, Format_S_T },
    { "teq", Special(Special_Function::Teq), gSpecialMask, Format_S_T },
    { "tne", Special(Special_Function::Tne), gSpecialMask, Format_S_T },
    { "dsll", Special(Special_Function::Dsll), gShiftMask, Format_D_T_Shift },
    { "dsrl", Special(Special_Function::Dsrl), gShiftMask, Format_D_T_Shift },
    { "dsra", Special(Special_Function::Dsra), gShiftMask, Format_D_T_Shift },
    { "dsll32", Special(Special_Function::Dsll32), gShiftMask, Format_D_T_Shift },
    { "dsrl32", Special(Special_Function::Dsrl32), gShiftMask, Format_D_T_Shift },
    { "dsra32", Special(Special_Function::Dsra32), gShiftMask, Format_D_T_Shift },

    { "bltz", Regimm(Regimm_Function::Bltz), gRegimmMask, Format_S_Branch },
    { "bgez", Regimm(Regimm_Function::Bgez), gRegimmMask, Format_S_Branch },
    { "bltzl", Regimm(Regimm_Function::Bltzl), gRegimmMask, Format_S_Branch },
    { "bgezl", Regimm(Regimm_Function::Bgezl), gRegimmMask, Format_S_Branch },
    { "tgei", Regimm(Regimm_Function::Tgei), gRegimmMask, Format_S_Immediate },
    { "tgeiu", Regimm(Regimm_Function::Tgeiu), gRegimmMask, Format_S_Immediate },
    { "tlti", Regimm(Regimm_Function::Tlti), gRegimmMask, Format_S_Immediate },
    { "tltiu", Regimm(Regimm_Function::Tltiu), gRegimmMask, Format_S_Immediate },
    { "teqi", Regimm(Regimm_Function::Teqi), gRegimmMask, Format_S_Immediate },
    { "tnei", Regimm(Regimm_Function::Tnei), gRegimmMask, Format_S_Immediate },
    { "bltzal", Regimm(Regimm_Function::Bltzal), gRegimmMask, Format_S_Branch },
    { "bgezal", Regimm(Regimm_Function::Bgezal), gRegimmMask, Format_S_Branch },
    { "bltzall", Regimm(Regimm_Function::Bltzall), gRegimmMask, Format_S_Branch },
    { "bgezall", Regimm(Regimm_Function::Bgezall), gRegimmMask, Format_S_Branch },

    { "j", Op(Opcode::J), gOpMask, Format_Jump },
    { "jal", Op(Opcode::Jal), gOpMask, Format_Jump },
    { "beq", Op(Opcode::Beq), gOpMask, Format_S_T_Branch },
    { "bne", Op(Opcode::Bne), gOpMask, Format_S_T_Branch },
    { "blez", Op(Opcode::Blez), 0xFC1F0000, Format_S_Branch },
    { "bgtz", Op(Opcode::Bgtz), 0xFC1F0000, Format_S_Branch },
    { "addi", Op(Opcode::Addi), gOpMask, Format_T_S_Signed },
    { "addiu", Op(Opcode::Addiu), gOpMask, Format_T_S_Signed },
    { "slti", Op(Opcode::Slti), gOpMask, Format_T_S_Signed },
    { "sltiu", Op(Opcode::Sltiu), gOpMask, Format_T_S_Signed },
    { "andi", Op(Opcode::Andi), gOpMask, Format_T_S_Unsigned },
    { "ori", Op(Opcode::Ori), gOpMask, Format_T_S_Unsigned },
    { "xori", Op(Opcode::Xori), gOpMask, Format_T_S_Unsigned },
    { "lui", Op(Opcode::Lui), 0xFFE00000, Format_T_Upper },
    { "beql", Op(Opcode::Beql), gOpMask, Format_S_T_Branch },
    { "bnel", Op(Opcode::Bnel), gOpMask, Format_S_T_Branch },
    { "blezl", Op(Opcode::Blezl), 0xFC1F0000, Format_S_Branch },
    { "bgtzl", Op(Opcode::Bgtzl), 0xFC1F0000, Format_S_Branch },
    { "daddi", Op(Opcode::Daddi), gOpMask, Format_T_S_Signed },
    { "daddiu", Op(Opcode::Daddiu), gOpMask, Format_T_S_Signed },
    { "ldl", Op(Opcode::Ldl), gOpMask, Format_T_Memory },
    { "ldr", Op(Opcode::Ldr), gOpMask, Format_T_Memory },
    { "lb", Op(Opcode::Lb), gOpMask, Format_T_Memory },
    { "lh", Op(Opcode::Lh), gOpMask, Format_T_Memory },
    { "lwl", Op(Opcode::Lwl), gOpMask, Format_T_Memory },
    { "lw", Op(Opcode::Lw), gOpMask, Format_T_Memory },
    { "lbu", Op(Opcode::Lbu), gOpMask, Format_T_Memory },
    { "lhu", Op(Opcode::Lhu), gOpMask, Format_T_Memory },
    { "lwr", Op(Opcode::Lwr), gOpMask, Format_T_Memory },
    { "lwu", Op(Opcode::Lwu), gOpMask, Format_T_Memory },
    { "sb", Op(Opcode::Sb), gOpMask, Format_T_Memory },
    { "sh", Op(Opcode::Sh), gOpMask, Format_T_Memory },
    { "swl", Op(Opcode::Swl), gOpMask, Format_T_Memory },
    { "sw", Op(Opcode::Sw), gOpMask, Format_T_Memory },
    { "sdl", Op(Opcode::Sdl), gOpMask, Format_T_Memory },
    { "sdr", Op(Opcode::Sdr), gOpMask, Format_T_Memory },
    { "swr", Op(Opcode::Swr), gOpMask, Format_T_Memory },
    { "cache", Op(Opcode::Cache), gOpMask, Format_Cache },
    { "ll", Op(Opcode::Ll), gOpMask, Format_T_Memory },
    { "lwc1", Op(Opcode::Lwc1), gOpMask, Format_Ft_Memory },
    { "lld", Op(Opcode::Lld), gOpMask, Format_T_Memory },
    { "ldc1", Op(Opcode::Ldc1), gOpMask, Format_Ft_Memory },
    { "ld", Op(Opcode::Ld), gOpMask, Format_T_Memory },
    { "sc", Op(Opcode::Sc), gOpMask, Format_T_Memory },
    { "swc1", Op(Opcode::Swc1), gOpMask, Format_Ft_Memory },
    { "scd", Op(Opcode::Scd), gOpMask, Format_T_Memory },
    { "sdc1", Op(Opcode::Sdc1), gOpMask, Format_Ft_Memory },
    { "sd", Op(Opcode::Sd), gOpMask, Format_T_Memory },

    { "mfc0", Cop0(Coprocessor_Operation::Move_From), gMoveMask, Format_T_Cop0 },
    { "dmfc0", Cop0(Coprocessor_Operation::Double_Move_From), gMoveMask, Format_T_Cop0 },
    { "mtc0", Cop0(Coprocessor_Operation::Move_To), gMoveMask, Format_T_Cop0 },
    { "dmtc0", Cop0(Coprocessor_Operation::Double_Move_To), gMoveMask, Format_T_Cop0 },
    { "tlbr", Cop0(Coprocessor_Operation::Function) | static_cast<u32>(Cop0_Function::Tlb_Read), 0xFFFFFFFF, Format_None },
    { "tlbwi", Cop0(Coprocessor_Operation::Function) | static_cast<u32>(Cop0_Function::Tlb_Write_Indexed), 0xFFFFFFFF, Format_None },
    { "tlbwr", Cop0(Coprocessor_Operation::Function) | static_cast<u32>(Cop0_Function::Tlb_Write_Random), 0xFFFFFFFF, Format_None },
    { "tlbp", Cop0(Coprocessor_Operation::Function) | static_cast<u32>(Cop0_Function::Tlb_Probe), 0xFFFFFFFF, Format_None },
    { "eret", Cop0(Coprocessor_Operation::Function) | static_cast<u32>(Cop0_Function::Exception_Return), 0xFFFFFFFF, Format_None },

    { "mfc1", Cop1(Coprocessor_Operation::Move_From), gMoveMask, Format_T_Fs },
    { "dmfc1", Cop1(Coprocessor_Operation::Double_Move_From), gMoveMask, Format_T_Fs },
    { "cfc1", Cop1(Coprocessor_Operation::Control_From), gMoveMask, Format_T_Fcr },
    { "mtc1", Cop1(Coprocessor_Operation::Move_To), gMoveMask, Format_T_Fs },
    { "dmtc1", Cop1(Coprocessor_Operation::Double_Move_To), gMoveMask, Format_T_Fs },
    { "ctc1", Cop1(Coprocessor_Operation::Control_To), gMoveMask, Format_T_Fcr },
    { "bc1f", Cop1(Coprocessor_Operation::Branch) | (static_cast<u32>(Coprocessor_Branch::False) << 16), 0xFFFF0000, Format_Branch },
    { "bc1t", Cop1(Coprocessor_Operation::Branch) | (static_cast<u32>(Coprocessor_Branch::True) << 16), 0xFFFF0000, Format_Branch },
    { "bc1fl", Cop1(Coprocessor_Operation::Branch) | (static_cast<u32>(Coprocessor_Branch::False_Likely) << 16), 0xFFFF0000, Format_Branch },
    { "bc1tl", Cop1(Coprocessor_Operation::Branch) | (static_cast<u32>(Coprocessor_Branch::True_Likely) << 16), 0xFFFF0000, Format_Branch },

    { "add.s", Float(Float_Format::Single, Float_Function::Add), 0xFFE0003F, Format_Fd_Fs_Ft },
    { "add.d", Float(Float_Format::Double, Float_Function::Add), 0xFFE0003F, Format_Fd_Fs_Ft },
    { "sub.s", Float(Float_Format::Single, Float_Function::Sub), 0xFFE0003F, Format_Fd_Fs_Ft },
    { "sub.d", Float(Float_Format::Double, Float_Function::Sub), 0xFFE0003F, Format_Fd_Fs_Ft },
    { "mul.s", Float(Float_Format::Single, Float_Function::Mul), 0xFFE0003F, Format_Fd_Fs_Ft },
    { "mul.d", Float(Float_Format::Double, Float_Function::Mul), 0xFFE0003F, Format_Fd_Fs_Ft },
    { "div.s", Float(Float_Format::Single, Float_Function::Div), 0xFFE0003F, Format_Fd_Fs_Ft },
    { "div.d", Float(Float_Format::Double, Float_Function::Div), 0xFFE0003F, Format_Fd_Fs_Ft },
    { "sqrt.s", Float(Float_Format::Single, Float_Function::Sqrt), 0xFFFF003F, Format_Fd_Fs },
    { "sqrt.d", Float(Float_Format::Double, Float_Function::Sqrt), 0xFFFF003F, Format_Fd_Fs },
    { "abs.s", Float(Float_Format::Single, Float_Function::Abs), 0xFFFF003F, Format_Fd_Fs },
    { "abs.d", Float(Float_Format::Double, Float_Function::Abs), 0xFFFF003F, Format_Fd_Fs },
    { "mov.s", Float(Float_Format::Single, Float_Function::Mov), 0xFFFF003F, Format_Fd_Fs },
    { "mov.d", Float(Float_Format::Double, Float_Function::Mov), 0xFFFF003F, Format_Fd_Fs },
    { "neg.s", Float(Float_Format::Single, Float_Function::Neg), 0xFFFF003F, Format_Fd_Fs },
    { "neg.d", Float(Float_Format::Double, Float_Function::Neg), 0xFFFF003F, Format_Fd_Fs },
    { "round.l.s", Float(Float_Format::Single, Float_Function::Round_l), 0xFFFF003F, Format_Fd_Fs },
    { "round.l.d", Float(Float_Format::Double, Float_Function::Round_l), 0xFFFF003F, Format_Fd_Fs },
    { "trunc.l.s", Float(Float_Format::Single, Float_Function::Trunc_l), 0xFFFF003F, Format_Fd_Fs },
    { "trunc.l.d", Float(Float_Format::Double, Float_Function::Trunc_l), 0xFFFF003F, Format_Fd_Fs },
    { "ceil.l.s", Float(Float_Format::Single, Float_Function::Ceil_l), 0xFFFF003F, Format_Fd_Fs },
    { "ceil.l.d", Float(Float_Format::Double, Float_Function::Ceil_l), 0xFFFF003F, Format_Fd_Fs },
    { "floor.l.s", Float(Float_Format::Single, Float_Function::Floor_l), 0xFFFF003F, Format_Fd_Fs },
    { "floor.l.d", Float(Float_Format::Double, Float_Function::Floor_l), 0xFFFF003F, Format_Fd_Fs },
    { "round.w.s", Float(Float_Format::Single, Float_Function::Round_w), 0xFFFF003F, Format_Fd_Fs },
    { "round.w.d", Float(Float_Format::Double, Float_Function::Round_w), 0xFFFF003F, Format_Fd_Fs },
    { "trunc.w.s", Float(Float_Format::Single, Float_Function::Trunc_w), 0xFFFF003F, Format_Fd_Fs },
    { "trunc.w.d", Float(Float_Format::Double, Float_Function::Trunc_w), 0xFFFF003F, Format_Fd_Fs },
    { "ceil.w.s", Float(Float_Format::Single, Float_Function::Ceil_w), 0xFFFF003F, Format_Fd_Fs },
    { "ceil.w.d", Float(Float_Format::Double, Float_Function::Ceil_w), 0xFFFF003F, Format_Fd_Fs },
    { "floor.w.s", Float(Float_Format::Single, Float_Function::Floor_w), 0xFFFF003F, Format_Fd_Fs },
    { "floor.w.d", Float(Float_Format::Double, Float_Function::Floor_w), 0xFFFF003F, Format_Fd_Fs },
    { "cvt.s.d", Float(Float_Format::Double, Float_Function::Cvt_s), 0xFFFF003F, Format_Fd_Fs },
    { "cvt.s.w", Float(Float_Format::Word, Float_Function::Cvt_s), 0xFFFF003F, Format_Fd_Fs },
    { "cvt.s.l", Float(Float_Format::Long, Float_Function::Cvt_s), 0xFFFF003F, Format_Fd_Fs },
    { "cvt.d.s", Float(Float_Format::Single, Float_Function::Cvt_d), 0xFFFF003F, Format_Fd_Fs },
    { "cvt.d.w", Float(Float_Format::Word, Float_Function::Cvt_d), 0xFFFF003F, Format_Fd_Fs },
    { "cvt.d.l", Float(Float_Format::Long, Float_Function::Cvt_d), 0xFFFF003F, Format_Fd_Fs },
    { "cvt.w.s", Float(Float_Format::Single, Float_Function::Cvt_w), 0xFFFF003F, Format_Fd_Fs },
    { "cvt.w.d", Float(Float_Format::Double, Float_Function::Cvt_w), 0xFFFF003F, Format_Fd_Fs },
    { "cvt.l.s", Float(Float_Format::Single, Float_Function::Cvt_l), 0xFFFF003F, Format_Fd_Fs },
    { "cvt.l.d", Float(Float_Format::Double, Float_Function::Cvt_l), 0xFFFF003F, Format_Fd_Fs },
    { "c.f.s", Float(Float_Format::Single, Float_Function::Compare_F), 0xFFE007FF, Format_Fs_Ft },
    { "c.f.d", Float(Float_Format::Double, Float_Function::Compare_F), 0xFFE007FF, Format_Fs_Ft },
    { "c.un.s", Float(Float_Format::Single, Float_Function::Compare_Un), 0xFFE007FF, Format_Fs_Ft },
    { "c.un.d", Float(Float_Format::Double, Float_Function::Compare_Un), 0xFFE007FF, Format_Fs_Ft },
    { "c.eq.s", Float(Float_Format::Single, Float_Function::Compare_Eq), 0xFFE007FF, Format_Fs_Ft },
    { "c.eq.d", Float(Float_Format::Double, Float_Function::Compare_Eq), 0xFFE007FF, Format_Fs_Ft },
    { "c.ueq.s", Float(Float_Format::Single, Float_Function::Compare_Ueq), 0xFFE007FF, Format_Fs_Ft },
    { "c.ueq.d", Float(Float_Format::Double, Float_Function::Compare_Ueq), 0xFFE007FF, Format_Fs_Ft },
    { "c.olt.s", Float(Float_Format::Single, Float_Function::Compare_Olt), 0xFFE007FF, Format_Fs_Ft },
    { "c.olt.d", Float(Float_Format::Double, Float_Function::Compare_Olt), 0xFFE007FF, Format_Fs_Ft },
    { "c.ult.s", Float(Float_Format::Single, Float_Function::Compare_Ult), 0xFFE007FF, Format_Fs_Ft },
    { "c.ult.d", Float(Float_Format::Double, Float_Function::Compare_Ult), 0xFFE007FF, Format_Fs_Ft },
    { "c.ole.s", Float(Float_Format::Single, Float_Function::Compare_Ole), 0xFFE007FF, Format_Fs_Ft },
    { "c.ole.d", Float(Float_Format::Double, Float_Function::Compare_Ole), 0xFFE007FF, Format_Fs_Ft },
    { "c.ule.s", Float(Float_Format::Single, Float_Function::Compare_Ule), 0xFFE007FF, Format_Fs_Ft },
    { "c.ule.d", Float(Float_Format::Double, Float_Function::Compare_Ule), 0xFFE007FF, Format_Fs_Ft },
    { "c.sf.s", Float(Float_Format::Single, Float_Function::Compare_Sf), 0xFFE007FF, Format_Fs_Ft },
    { "c.sf.d", Float(Float_Format::Double, Float_Function::Compare_Sf), 0xFFE007FF, Format_Fs_Ft },
    { "c.ngle.s", Float(Float_Format::Single, Float_Function::Compare_Ngle), 0xFFE007FF, Format_Fs_Ft },
    { "c.ngle.d", Float(Float_Format::Double, Float_Function::Compare_Ngle), 0xFFE007FF, Format_Fs_Ft },
    { "c.seq.s", Float(Float_Format::Single, Float_Function::Compare_Seq), 0xFFE007FF, Format_Fs_Ft },
    { "c.seq.d", Float(Float_Format::Double, Float_Function::Compare_Seq), 0xFFE007FF, Format_Fs_Ft },
    { "c.ngl.s", Float(Float_Format::Single, Float_Function::Compare_Ngl), 0xFFE007FF, Format_Fs_Ft },
    { "c.ngl.d", Float(Float_Format::Double, Float_Function::Compare_Ngl), 0xFFE007FF, Format_Fs_Ft },
    { "c.lt.s", Float(Float_Format::Single, Float_Function::Compare_Lt), 0xFFE007FF, Format_Fs_Ft },
    { "c.lt.d", Float(Float_Format::Double, Float_Function::Compare_Lt), 0xFFE007FF, Format_Fs_Ft },
    { "c.nge.s", Float(Float_Format::Single, Float_Function::Compare_Nge), 0xFFE007FF, Format_Fs_Ft },
    { "c.nge.d", Float(Float_Format::Double, Float_Function::Compare_Nge), 0xFFE007FF, Format_Fs_Ft },
    { "c.le.s", Float(Float_Format::Single, Float_Function::Compare_Le), 0xFFE007FF, Format_Fs_Ft },
    { "c.le.d", Float(Float_Format::Double, Float_Function::Compare_Le), 0xFFE007FF, Format_Fs_Ft },
    { "c.ngt.s", Float(Float_Format::Single, Float_Function::Compare_Ngt), 0xFFE007FF, Format_Fs_Ft },
    { "c.ngt.d", Float(Float_Format::Double, Float_Function::Compare_Ngt), 0xFFE007FF, Format_Fs_Ft },
};

} // namespace

bool Names_Equal(const char* name, u32 length, const char* candidate) {
    u32 candidate_length = static_cast<u32>(strlen(candidate));
    if (candidate_length != length) {
        return false;
    }

    for (u32 index = 0; index < length; index++) {
        char left = name[index];
        char right = candidate[index];

        if (left >= 'A' && left <= 'Z') {
            left = static_cast<char>(left + 32);
        }
        if (right >= 'A' && right <= 'Z') {
            right = static_cast<char>(right + 32);
        }

        if (left != right) {
            return false;
        }
    }
    return true;
}

const Opcode_Entry* Find_Mnemonic(const char* name, u32 length) {
    for (const Opcode_Entry& entry : gOpcodes) {
        if (Names_Equal(name, length, entry.mnemonic)) {
            return &entry;
        }
    }
    return nullptr;
}

const Opcode_Entry* Find_Instruction(u32 instruction) {
    for (const Opcode_Entry& entry : gOpcodes) {
        if ((instruction & entry.mask) == entry.match) {
            return &entry;
        }
    }
    return nullptr;
}

const char* const gGprNames[32] = {
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0",   "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
};

const char* const gCop0Names[32] = {
    "Index",   "Random", "EntryLo0", "EntryLo1", "Context", "PageMask", "Wired",  "$7",      "BadVAddr", "Count",    "EntryHi",
    "Compare", "Status", "Cause",    "EPC",      "PRId",    "Config",   "LLAddr", "WatchLo", "WatchHi",  "XContext", "$21",
    "$22",     "$23",    "$24",      "$25",      "PErr",    "CacheErr", "TagLo",  "TagHi",   "ErrorEPC", "$31",
};

} // namespace N64
