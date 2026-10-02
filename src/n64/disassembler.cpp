#include "disassembler.h"
#include "opcodes.h"

#include <stdio.h>

namespace N64 {

namespace {

const char* Signed_Hex(s32 value, char* out, size_t capacity) {
    snprintf(out, capacity, value < 0 ? "-0x%X" : "0x%X", value < 0 ? static_cast<u32>(-static_cast<s64>(value)) : static_cast<u32>(value));
    return out;
}

} // namespace

u32 Disassembler::Branch_Target() const {
    return address + 4 + (static_cast<u32>(static_cast<s16>(word & 0xFFFF)) << 2);
}

u32 Disassembler::Jump_Target() const {
    return ((address + 4) & 0xF0000000u) | ((word & 0x3FFFFFF) << 2);
}

size_t Disassembler::Format(char* out, size_t capacity) const {
    const Opcode_Entry* entry = Find_Instruction(word);
    u32 rs = (word >> 21) & 31;
    u32 rt = (word >> 16) & 31;
    u32 rd = (word >> 11) & 31;
    u32 sa = (word >> 6) & 31;
    s32 simm = static_cast<s16>(word & 0xFFFF);
    u32 uimm = word & 0xFFFF;
    u32 target = Branch_Target();
    u32 jump = Jump_Target();
    char number[16];
    char operands[64] = "";
    s32 length;

    if (entry == nullptr) {
        length = snprintf(out, capacity, ".word 0x%08X", word);
        return length > 0 ? static_cast<size_t>(length) : 0;
    }

    const char* mnemonic = entry->mnemonic;
    switch (entry->format) {
    case Format_None:
        break;
    case Format_D_S_T:
        if (rt == 0 && (entry->match == Special(Special_Function::Addu) || entry->match == Special(Special_Function::Or) || entry->match == Special(Special_Function::Daddu))) {
            mnemonic = "move";
            snprintf(operands, sizeof(operands), "%s, %s", gGprNames[rd], gGprNames[rs]);
            break;
        }
        snprintf(operands, sizeof(operands), "%s, %s, %s", gGprNames[rd], gGprNames[rs], gGprNames[rt]);
        break;
    case Format_D_T_S:
        snprintf(operands, sizeof(operands), "%s, %s, %s", gGprNames[rd], gGprNames[rt], gGprNames[rs]);
        break;
    case Format_D_T_Shift:
        snprintf(operands, sizeof(operands), "%s, %s, %u", gGprNames[rd], gGprNames[rt], sa);
        break;
    case Format_S_T:
        snprintf(operands, sizeof(operands), "%s, %s", gGprNames[rs], gGprNames[rt]);
        break;
    case Format_Fs_Ft:
        snprintf(operands, sizeof(operands), "f%u, f%u", rd, rt);
        break;
    case Format_D:
        snprintf(operands, sizeof(operands), "%s", gGprNames[rd]);
        break;
    case Format_Jalr:
        if (rd != 31) {
            snprintf(operands, sizeof(operands), "%s, %s", gGprNames[rd], gGprNames[rs]);
            break;
        }
        [[fallthrough]];
    case Format_S:
        snprintf(operands, sizeof(operands), "%s", gGprNames[rs]);
        break;
    case Format_T_S_Signed:
        if (rs == 0 && (entry->match == Op(Opcode::Addiu) || entry->match == Op(Opcode::Daddiu))) {
            mnemonic = "li";
            snprintf(operands, sizeof(operands), "%s, %s", gGprNames[rt], Signed_Hex(simm, number, sizeof(number)));
            break;
        }
        snprintf(operands, sizeof(operands), "%s, %s, %s", gGprNames[rt], gGprNames[rs], Signed_Hex(simm, number, sizeof(number)));
        break;
    case Format_T_S_Unsigned:
        if (rs == 0 && entry->match == Op(Opcode::Ori)) {
            mnemonic = "li";
            snprintf(operands, sizeof(operands), "%s, 0x%X", gGprNames[rt], uimm);
            break;
        }
        snprintf(operands, sizeof(operands), "%s, %s, 0x%X", gGprNames[rt], gGprNames[rs], uimm);
        break;
    case Format_T_Upper:
        snprintf(operands, sizeof(operands), "%s, 0x%X", gGprNames[rt], uimm);
        break;
    case Format_T_Memory:
        snprintf(operands, sizeof(operands), "%s, %s(%s)", gGprNames[rt], Signed_Hex(simm, number, sizeof(number)), gGprNames[rs]);
        break;
    case Format_Ft_Memory:
        snprintf(operands, sizeof(operands), "f%u, %s(%s)", rt, Signed_Hex(simm, number, sizeof(number)), gGprNames[rs]);
        break;
    case Format_Cache:
        snprintf(operands, sizeof(operands), "0x%X, %s(%s)", rt, Signed_Hex(simm, number, sizeof(number)), gGprNames[rs]);
        break;
    case Format_S_T_Branch:
        if (rs == 0 && rt == 0 && entry->match == Op(Opcode::Beq)) {
            mnemonic = "b";
            snprintf(operands, sizeof(operands), "0x%08X", target);
            break;
        }

        if (rt == 0 && (entry->match == Op(Opcode::Beq) || entry->match == Op(Opcode::Bne))) {
            mnemonic = entry->match == Op(Opcode::Beq) ? "beqz" : "bnez";
            snprintf(operands, sizeof(operands), "%s, 0x%08X", gGprNames[rs], target);
            break;
        }

        snprintf(operands, sizeof(operands), "%s, %s, 0x%08X", gGprNames[rs], gGprNames[rt], target);
        break;
    case Format_S_Branch:
        snprintf(operands, sizeof(operands), "%s, 0x%08X", gGprNames[rs], target);
        break;
    case Format_Branch:
        snprintf(operands, sizeof(operands), "0x%08X", target);
        break;
    case Format_Jump:
        snprintf(operands, sizeof(operands), "0x%08X", jump);
        break;
    case Format_Code:
        if (((word >> 6) & 0xFFFFF) == 0) {
            break;
        }
        [[fallthrough]];
    case Format_Hypercall:
        snprintf(operands, sizeof(operands), "0x%X", (word >> 6) & 0xFFFFF);
        break;
    case Format_T_Cop0:
        snprintf(operands, sizeof(operands), "%s, %s", gGprNames[rt], gCop0Names[rd]);
        break;
    case Format_T_Fs:
        snprintf(operands, sizeof(operands), "%s, f%u", gGprNames[rt], rd);
        break;
    case Format_T_Fcr:
        snprintf(operands, sizeof(operands), "%s, $%u", gGprNames[rt], rd);
        break;
    case Format_Fd_Fs_Ft:
        snprintf(operands, sizeof(operands), "f%u, f%u, f%u", sa, rd, rt);
        break;
    case Format_Fd_Fs:
        snprintf(operands, sizeof(operands), "f%u, f%u", sa, rd);
        break;
    case Format_S_Immediate:
        snprintf(operands, sizeof(operands), "%s, %s", gGprNames[rs], Signed_Hex(simm, number, sizeof(number)));
        break;
    }
    
    length = operands[0] != '\0' ? snprintf(out, capacity, "%-8s%s", mnemonic, operands) : snprintf(out, capacity, "%s", mnemonic);
    return length > 0 ? static_cast<size_t>(length) : 0;
}

Instruction_Info Disassembler::Analyze() const {
    Instruction_Info info = {};
    if (Find_Instruction(word) == nullptr) {
        info.control = Control_Other;
        return info;
    }
    Opcode opcode = static_cast<Opcode>(word >> 26);
    u32 target_register = (word >> 16) & 31;

    switch (opcode) {
    case Opcode::Special:
        switch (static_cast<Special_Function>(word & 0x3F)) {
        case Special_Function::Jr:
        case Special_Function::Jalr:
            info.control = Control_Jump_Register;
            break;
        case Special_Function::Syscall:
        case Special_Function::Break:
        case Special_Function::Tge:
        case Special_Function::Tgeu:
        case Special_Function::Tlt:
        case Special_Function::Tltu:
        case Special_Function::Teq:
        case Special_Function::Reserved_Trap:
        case Special_Function::Tne:
            info.control = Control_Other;
            break;
        default:
            break;
        }
        break;
    case Opcode::Regimm:
        switch (static_cast<Regimm_Function>(target_register)) {
        case Regimm_Function::Bltz:
        case Regimm_Function::Bgez:
            info.control = Control_Branch;
            break;
        case Regimm_Function::Bltzl:
        case Regimm_Function::Bgezl:
            info.control = Control_Branch_Likely;
            break;
        case Regimm_Function::Bltzal:
        case Regimm_Function::Bgezal:
        case Regimm_Function::Bltzall:
        case Regimm_Function::Bgezall:
            info.control = Control_Branch_Link;
            break;
        default:
            info.control = Control_Other;
            break;
        }
        break;
    case Opcode::J:
    case Opcode::Jal:
        info.control = opcode == Opcode::J ? Control_Jump : Control_Jump_Link;
        break;
    case Opcode::Beq:
    case Opcode::Bne:
    case Opcode::Blez:
    case Opcode::Bgtz:
        info.control = Control_Branch;
        break;
    case Opcode::Beql:
    case Opcode::Bnel:
    case Opcode::Blezl:
    case Opcode::Bgtzl:
        info.control = Control_Branch_Likely;
        break;
    case Opcode::Cop0:
        if (word == (Cop0(Coprocessor_Operation::Function) | static_cast<u32>(Cop0_Function::Exception_Return))) {
            info.control = Control_Other;
        }
        break;
    case Opcode::Cop1:
        if (static_cast<Coprocessor_Operation>((word >> 21) & 31) == Coprocessor_Operation::Branch) {
            switch (static_cast<Coprocessor_Branch>(target_register & 3)) {
            case Coprocessor_Branch::False:
            case Coprocessor_Branch::True:
                info.control = Control_Branch;
                break;
            case Coprocessor_Branch::False_Likely:
            case Coprocessor_Branch::True_Likely:
                info.control = Control_Branch_Likely;
                break;
            }
        }
        break;
    default:
        break;
    }
    switch (info.control) {
    case Control_Branch:
    case Control_Branch_Likely:
    case Control_Branch_Link:
        info.target = Branch_Target();
        break;
    case Control_Jump:
    case Control_Jump_Link:
        info.target = Jump_Target();
        break;
    case Control_Jump_Register:
        break;
    case Control_None:
    case Control_Other:
        return info;
    }
    info.hasDelaySlot = true;
    return info;
}

} // namespace N64
