#pragma once

#include <stddef.h>
#include <stdint.h>

namespace N64 {

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

struct Memory_View {
    const u8* bytes = nullptr;
    u32 address = 0;
    u32 size = 0;
    bool wordSwapped = false;
};

} // namespace N64
