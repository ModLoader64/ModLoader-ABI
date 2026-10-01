#pragma once

#include "instruction.h"

namespace N64::Detail {

constexpr u32 gMaxRelocatedWords = 16;

u32 Relocate_Displaced(const u32 original[3], u32 site, u32 destination, u32 out_words[gMaxRelocatedWords]);
bool Delay_Slot_Valid(u32 word, u32 address);
bool Payload_Valid(const u8* bytes, u32 size, u32 origin);
bool Detour_Words(u32 site, u32 destination, u32 out_words[2], u32 delay_word = gNop);

}
