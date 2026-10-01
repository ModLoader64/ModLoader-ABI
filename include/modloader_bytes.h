#pragma once

#include <stddef.h>
#include <stdint.h>

namespace ModLoader::Bytes {

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

inline u64 Read_Integer(const void* bytes, u32 size, bool big_endian) {
    const u8* input = static_cast<const u8*>(bytes);
    u64 value = 0;
    for (u32 index = 0; index < size; index++) {
        value = (value << 8) | input[big_endian ? index : size - 1 - index];
    }
    return value;
}

inline void Write_Integer(void* bytes, u64 value, u32 size, bool big_endian) {
    u8* output = static_cast<u8*>(bytes);
    for (u32 index = 0; index < size; index++) {
        output[big_endian ? size - 1 - index : index] = static_cast<u8>(value);
        value >>= 8;
    }
}

inline u16 Read_Be16(const void* bytes) {
    return static_cast<u16>(Read_Integer(bytes, sizeof(u16), true));
}

inline u32 Read_Be32(const void* bytes) {
    return static_cast<u32>(Read_Integer(bytes, sizeof(u32), true));
}

inline u16 Read_Le16(const void* bytes) {
    return static_cast<u16>(Read_Integer(bytes, sizeof(u16), false));
}

inline u32 Read_Le32(const void* bytes) {
    return static_cast<u32>(Read_Integer(bytes, sizeof(u32), false));
}

inline void Write_Be16(void* bytes, u16 value) {
    Write_Integer(bytes, value, sizeof(value), true);
}

inline void Write_Be32(void* bytes, u32 value) {
    Write_Integer(bytes, value, sizeof(value), true);
}

inline void Write_Be64(void* bytes, u64 value) {
    Write_Integer(bytes, value, sizeof(value), true);
}

inline void Write_Le16(void* bytes, u16 value) {
    Write_Integer(bytes, value, sizeof(value), false);
}

inline void Write_Le32(void* bytes, u32 value) {
    Write_Integer(bytes, value, sizeof(value), false);
}

inline void Write_Le64(void* bytes, u64 value) {
    Write_Integer(bytes, value, sizeof(value), false);
}

inline void Write_Be32(void* bytes, const u32* words, size_t count) {
    u8* output = static_cast<u8*>(bytes);
    for (size_t index = 0; index < count; index++) {
        Write_Be32(output, words[index]);
        output += sizeof(u32);
    }
}

inline void Hex_Encode(const void* bytes, size_t size, char* output) {
    constexpr char gDigits[] = "0123456789abcdef";
    const u8* input = static_cast<const u8*>(bytes);
    for (size_t index = 0; index < size; index++) {
        output[index * 2] = gDigits[input[index] >> 4];
        output[index * 2 + 1] = gDigits[input[index] & 15];
    }
}

}
