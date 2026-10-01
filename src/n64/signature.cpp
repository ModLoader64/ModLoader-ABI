#include "signature.h"

#include <stdlib.h>
#include <string.h>

namespace N64 {

static bool Is_Whitespace(char character) {
    return character == ' ' || character == '\t' || character == '\n' || character == '\r';
}

Signature::Signature(const char* text) {
    Parse(text);
}

Signature::~Signature() {
    Destroy();
}

bool Signature::Parse(const char* text) {
    if (text == nullptr) {
        return false;
    }

    u64 nibbles = 0;
    for (const char* cursor = text; *cursor != '\0'; cursor++) {
        char character = *cursor;
        if (Is_Whitespace(character)) {
            continue;
        }

        bool hex = (character >= '0' && character <= '9') || (character >= 'a' && character <= 'f') || (character >= 'A' && character <= 'F');
        if ((!hex && character != '?' && character != '.') || nibbles == static_cast<u64>(UINT32_MAX) * 2) {
            return false;
        }
        nibbles++;
    }

    if (nibbles == 0 || (nibbles & 1) != 0 || nibbles > SIZE_MAX) {
        return false;
    }

    u8* parsed = static_cast<u8*>(malloc(static_cast<size_t>(nibbles)));
    if (parsed == nullptr) {
        return false;
    }

    memset(parsed, 0, static_cast<size_t>(nibbles));
    u32 parsed_length = static_cast<u32>(nibbles / 2);
    u8* mask = parsed + parsed_length;
    u64 index = 0;
    for (const char* cursor = text; *cursor != '\0'; cursor++) {
        char character = *cursor;
        if (Is_Whitespace(character)) {
            continue;
        }

        u32 byte_index = static_cast<u32>(index / 2);
        bool high = (index & 1) == 0;
        if (character != '?' && character != '.') {
            u32 value = character <= '9' ? static_cast<u32>(character - '0') : static_cast<u32>((character | 0x20) - 'a' + 10);
            parsed[byte_index] |= static_cast<u8>(high ? value << 4 : value);
            mask[byte_index] |= high ? 0xF0 : 0x0F;
        }
        index++;
    }

    Destroy();
    bytes = parsed;
    length = parsed_length;
    return true;
}

void Signature::Destroy() {
    free(bytes);
    bytes = nullptr;
    length = 0;
}

bool Signature::Is_Valid() const {
    return bytes != nullptr;
}

u32 Signature::Size() const {
    return length;
}

bool Signature::Find_With(const Memory_View& view, u32* out_address, u32 start, u32 alignment, bool unique) const {
    if (!Is_Valid() || view.bytes == nullptr || out_address == nullptr || length > view.size || start > view.size - length ||
        static_cast<u64>(view.address) + view.size > static_cast<u64>(UINT32_MAX) + 1 ||
        (view.wordSwapped && ((view.address | view.size) & 3) != 0)) {
        return false;
    }

    if (alignment == 0) {
        alignment = 1;
    }

    u32 last = view.size - length;
    u32 padding = static_cast<u32>((alignment - (static_cast<u64>(view.address) + start) % alignment) % alignment);
    if (padding > last - start) {
        return false;
    }

    const u8* mask = bytes + length;
    u32 byte_xor = view.wordSwapped ? 3 : 0;
    u32 address = 0;
    bool found = false;
    for (u32 offset = start + padding;; offset += alignment) {
        bool match = true;
        for (u32 index = 0; index < length; index++) {
            if ((view.bytes[(offset + index) ^ byte_xor] & mask[index]) != bytes[index]) {
                match = false;
                break;
            }
        }

        if (match) {
            if (found) {
                return false;
            }
            address = view.address + offset;
            found = true;
            if (!unique) {
                break;
            }
        }

        if (alignment > last - offset) {
            break;
        }
    }
    
    if (found) {
        *out_address = address;
    }
    return found;
}

bool Signature::Find(const Memory_View& view, u32* out_address, u32 start, u32 alignment) const {
    return Find_With(view, out_address, start, alignment, false);
}

bool Signature::Find_Unique(const Memory_View& view, u32* out_address, u32 start, u32 alignment) const {
    return Find_With(view, out_address, start, alignment, true);
}

}
