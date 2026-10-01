#include "relocate.h"
#include "disassembler.h"
#include "modloader_bytes.h"

namespace N64::Detail {

namespace {

bool Fits_Output(u32 address, u32 count) {
    return static_cast<u64>(address) + static_cast<u64>(count) * sizeof(u32) <= (u64{1} << 32);
}

bool Same_Region(u32 left, u32 right) {
    return (left & 0xF0000000u) == (right & 0xF0000000u);
}

void Load_Link(u32 target_register, u32 return_address, u32* out_words) {
    out_words[0] = (static_cast<u32>(Opcode::Lui) << 26) | (target_register << 16) | (return_address >> 16);
    out_words[1] = (static_cast<u32>(Opcode::Ori) << 26) | (target_register << 21) | (target_register << 16) | (return_address & 0xFFFFu);
}

u32 Relocate_Transfer(u32 word, const Instruction_Info& info, u32 delay_slot, u32 destination, u32 resume_address, u32 site, u32* out_words) {
    if (info.control != Control_Jump_Register && info.target - site < 8) {
        return 0;
    }
    switch (info.control) {
    case Control_Jump:
        if (!Fits_Output(destination, 2) || !Same_Region(info.target, destination + 4)) {
            return 0;
        }
        out_words[0] = Encode_Jump(info.target);
        out_words[1] = delay_slot;
        return 2;
    case Control_Jump_Link:
        if (!Fits_Output(destination, 4) || !Same_Region(info.target, destination + 12)) {
            return 0;
        }
        Load_Link(31, resume_address, out_words);
        out_words[2] = Encode_Jump(info.target);
        out_words[3] = delay_slot;
        return 4;
    case Control_Jump_Register: {
        u32 source_register = (word >> 21) & 31;
        u32 link_register = static_cast<Special_Function>(word & 0x3F) == Special_Function::Jalr ? (word >> 11) & 31 : 0;

        if (link_register == 0) {
            if (!Fits_Output(destination, 2)) {
                return 0;
            }
            out_words[0] = word;
            out_words[1] = delay_slot;
            return 2;
        }
        if (link_register == source_register || !Fits_Output(destination, 4)) {
            return 0;
        }
        Load_Link(link_register, resume_address, out_words);
        out_words[2] = (source_register << 21) | static_cast<u32>(Special_Function::Jr);
        out_words[3] = delay_slot;
        return 4;
    }
    case Control_Branch:
    case Control_Branch_Likely:
        if (!Fits_Output(destination, 6) || !Same_Region(info.target, destination + 20) || !Same_Region(resume_address, destination + 12)) {
            return 0;
        }
        out_words[0] = (word & 0xFFFF0000u) | 3;
        out_words[1] = delay_slot;
        out_words[2] = Encode_Jump(resume_address);
        out_words[3] = gNop;
        out_words[4] = Encode_Jump(info.target);
        out_words[5] = gNop;
        return 6;
    case Control_Branch_Link: {
        if (!Fits_Output(destination, 10) || !Same_Region(info.target, destination + 36) || !Same_Region(resume_address, destination + 20)) {
            return 0;
        }

        out_words[0] = (word & 0xFFE10000u) | 5;
        out_words[1] = gNop;
        Load_Link(31, resume_address, out_words + 2);
        out_words[4] = Encode_Jump(resume_address);
        Regimm_Function function = static_cast<Regimm_Function>((word >> 16) & 31);
        bool likely = function == Regimm_Function::Bltzall || function == Regimm_Function::Bgezall;
        out_words[5] = likely ? gNop : delay_slot;
        Load_Link(31, resume_address, out_words + 6);
        out_words[8] = Encode_Jump(info.target);
        out_words[9] = delay_slot;
        return 10;
    }
    default:
        return 0;
    }
}

} // namespace

bool Delay_Slot_Valid(u32 word, u32 address) {
    Special_Function function = static_cast<Special_Function>(word & 0x3F);
    bool hypercall = static_cast<Opcode>(word >> 26) == Opcode::Special && (function == Special_Function::Hypercall || function == Special_Function::Hypercallr);
    return !hypercall && Disassembler(word, address).Analyze().control == Control_None;
}

bool Payload_Valid(const u8* bytes, u32 size, u32 origin) {
    if (bytes == nullptr || size == 0 || ((origin | size) & 3) != 0 || !Fits_Output(origin, size / 4)) {
        return false;
    }

    for (u32 offset = 0; offset < size; offset += 4) {
        u32 address = origin + offset;
        if (Disassembler(ModLoader::Bytes::Read_Be32(bytes + offset), address).Analyze().hasDelaySlot) {
            if (size - offset < 8 || !Delay_Slot_Valid(ModLoader::Bytes::Read_Be32(bytes + offset + 4), address + 4)) {
                return false;
            }
            offset += 4;
        }
    }

    return true;
}

u32 Relocate_Displaced(const u32 original[3], u32 site, u32 destination, u32 out_words[gMaxRelocatedWords]) {
    if (original == nullptr || out_words == nullptr || ((site | destination) & 3) != 0 || site > UINT32_MAX - 8 || !Fits_Output(destination, 2)) {
        return 0;
    }

    Instruction_Info first = Disassembler(original[0], site).Analyze();
    Instruction_Info second = Disassembler(original[1], site + 4).Analyze();

    if (first.control == Control_Other || second.control == Control_Other) {
        return 0;
    }

    if (first.hasDelaySlot) {
        if (!Delay_Slot_Valid(original[1], site + 4)) {
            return 0;
        }
        return Relocate_Transfer(original[0], first, original[1], destination, site + 8, site, out_words);
    }

    if (second.hasDelaySlot) {
        if (site > UINT32_MAX - 12 || !Fits_Output(destination, 3) || !Delay_Slot_Valid(original[2], site + 8)) {
            return 0;
        }
        u32 count = Relocate_Transfer(original[1], second, original[2], destination + 4, site + 12, site, out_words + 1);
        if (count == 0) {
            return 0;
        }
        out_words[0] = original[0];
        return count + 1;
    }

    if (!Fits_Output(destination, 4) || !Same_Region(site + 8, destination + 12)) {
        return 0;
    }

    out_words[0] = original[0];
    out_words[1] = original[1];
    out_words[2] = Encode_Jump(site + 8);
    out_words[3] = gNop;
    return 4;
}

bool Detour_Words(u32 site, u32 destination, u32 out_words[2], u32 delay_word) {
    if (out_words == nullptr || ((site | destination) & 3) != 0 || !Fits_Output(site, 2) || !Same_Region(destination, site + 4) || !Delay_Slot_Valid(delay_word, site + 4)) {
        return false;
    }
    out_words[0] = Encode_Jump(destination);
    out_words[1] = delay_word;
    return true;
}

} // namespace N64::Detail
