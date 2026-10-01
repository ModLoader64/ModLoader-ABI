#pragma once

#include "assembler.h"
#include "modloader_n64_patch.h"
#include "signature.h"

namespace N64 {

enum class Patch_Mode : u32 {
    Inline = MODLOADER_N64_PATCH_INLINE,
    Detour = MODLOADER_N64_PATCH_DETOUR,
};

enum class Patch_Preserve : u32 {
    None = MODLOADER_N64_PATCH_PRESERVE_NONE,
    Source = MODLOADER_N64_PATCH_PRESERVE_SOURCE,
    Delay_Slot = MODLOADER_N64_PATCH_PRESERVE_DELAY_SLOT,
};

struct Patch_Bytes {
    const u8* data;
    u32 size;
};

class Patch {
public:
    Patch() = default;
    ~Patch();
    Patch(const Patch&) = delete;
    Patch& operator=(const Patch&) = delete;

    bool Create_Hook(const Patch_Memory& memory, u32 address, u32 destination, Patch_Preserve preserve = Patch_Preserve::None);
    bool Create_Hook(const Patch_Memory& memory, const Signature& signature, const Memory_View& search, u32 destination, Patch_Preserve preserve = Patch_Preserve::None);
    bool Create_Hypercall(const Patch_Memory& memory, u32 address, u32 id, Patch_Mode mode = Patch_Mode::Detour, Patch_Preserve preserve = Patch_Preserve::None);
    bool Create_Assembly(const Patch_Memory& memory, u32 address, const char* source, Patch_Mode mode = Patch_Mode::Detour, Patch_Preserve preserve = Patch_Preserve::None, Symbol_Resolver resolver = nullptr, void* resolver_context = nullptr);
    bool Create_Bytes(const Patch_Memory& memory, u32 address, const u8* bytes, u32 size);
    bool Create_Callback(const Patch_Memory& memory, u32 address, u32 entry_id, u32 exit_id);
    bool Enable();
    bool Disable();
    bool Destroy();

    bool Is_Created() const {
        return created;
    }

    bool Is_Enabled() const {
        return enabled;
    }

    Patch_Mode Mode() const {
        return mode;
    }

    u32 Address() const {
        return address;
    }

    u32 Destination() const {
        return destination;
    }

    u32 Trampoline() const {
        return trampoline;
    }

    u32 After_Address() const {
        return afterAddress;
    }

    Patch_Bytes Original_Bytes() const {
        return { originalBytes, patchSize };
    }

    Patch_Bytes Patched_Bytes() const {
        return { patchedBytes, patchSize };
    }

    Patch_Bytes Destination_Bytes() const {
        return { destinationBytes, destinationSize };
    }

private:
    bool Begin(const Patch_Memory& memory, u32 address, Patch_Mode mode, Patch_Preserve preserve, u32 payload_size, bool direct = false, u32 destination = 0, u32 exit_id = 0);
    bool Finish();
    bool Prepare(const u8* original, u8* patched, u8* output, u32* size, u32* after) const;
    u32 Installed_Size(const u8* bytes) const;
    bool Write_Destination(const u8* bytes, u32 size) const;
    void Clear();

    Patch_Memory memory = {};
    Patch_Mode mode = Patch_Mode::Inline;
    Patch_Preserve preserve = Patch_Preserve::None;
    u32 address = 0;
    u32 destination = 0;
    u32 trampoline = 0;
    u32 afterAddress = 0;
    u32 patchSize = 0;
    u32 originalSize = 0;
    u32 payloadSize = 0;
    u32 destinationSize = 0;
    u32 destinationCapacity = 0;
    u8* originalBytes = nullptr;
    u8* patchedBytes = nullptr;
    u8* destinationBytes = nullptr;
    u8* currentBytes = nullptr;
    u32 exitId = 0;
    bool direct = false;
    bool created = false;
    bool enabled = false;
};

} // namespace N64
