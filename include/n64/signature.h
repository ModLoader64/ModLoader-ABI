#pragma once

#include "types.h"

namespace N64 {

class Signature {
public:
    Signature() = default;
    explicit Signature(const char* text);
    ~Signature();
    Signature(const Signature&) = delete;
    Signature& operator=(const Signature&) = delete;

    // hex nibbles; ?/. are wildcards
    bool Parse(const char* text);
    void Destroy();
    bool Is_Valid() const;
    u32 Size() const;
    bool Find(const Memory_View& view, u32* out_address, u32 start = 0, u32 alignment = 4) const;
    bool Find_Unique(const Memory_View& view, u32* out_address, u32 start = 0, u32 alignment = 4) const;

private:
    bool Find_With(const Memory_View& view, u32* out_address, u32 start, u32 alignment, bool unique) const;

    u8* bytes = nullptr;
    u32 length = 0;
};

} // namespace N64
