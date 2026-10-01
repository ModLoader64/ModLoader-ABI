#include "patch.h"
#include "disassembler.h"
#include "relocate.h"
#include "modloader_bytes.h"

#include <stdlib.h>
#include <string.h>

namespace N64 {

namespace {

bool Entry_Valid(const Patch_Memory& memory, u32 address) {
    u8 previous[4];
    return address < 4 || !memory.read(memory.context, address - 4, previous, sizeof(previous)) || !Disassembler(ModLoader::Bytes::Read_Be32(previous), address - 4).Analyze().hasDelaySlot;
}

bool Range_Valid(u32 address, u32 size) {
    return size != 0 && static_cast<u64>(address) + size <= (u64{1} << 32);
}

} // namespace

Patch::~Patch() {
    free(originalBytes);
}

bool Patch::Begin(const Patch_Memory& provider, u32 site, Patch_Mode placement, Patch_Preserve preservation, u32 payload_size, bool external, u32 target, u32 exit_id) {
    if (created || provider.read == nullptr || provider.write == nullptr || (site & 3) != 0 ||
        placement > Patch_Mode::Detour || preservation > Patch_Preserve::Delay_Slot || (payload_size & 3) != 0 ||
        (placement == Patch_Mode::Inline && (preservation != Patch_Preserve::None || payload_size == 0 || external))) {
        return false;
    }

    u32 patch_size = placement == Patch_Mode::Inline ? payload_size : 8;
    u32 original_size = preservation == Patch_Preserve::Source ? 12 : patch_size;
    bool needs_destination = placement == Patch_Mode::Detour && (!external || preservation == Patch_Preserve::Source);
    u64 capacity = needs_destination ? static_cast<u64>(payload_size) + (Detail::gMaxRelocatedWords + 2) * sizeof(u32) : 0;
    u64 storage_size = static_cast<u64>(original_size) * 2 + patch_size + capacity;
    u32 words[2];
    if (!Range_Valid(site, original_size) || capacity > UINT32_MAX || storage_size > SIZE_MAX ||
        (needs_destination && (provider.allocate == nullptr || provider.release == nullptr)) ||
        (external && (!Detail::Detour_Words(site, target, words) || target - site < patch_size)) ||
        (!external && placement == Patch_Mode::Detour && site > UINT32_MAX - 8) || !Entry_Valid(provider, site)) {
        return false;
    }

    originalBytes = static_cast<u8*>(malloc(static_cast<size_t>(storage_size)));
    if (originalBytes == nullptr) {
        return false;
    }

    memory = provider;
    mode = placement;
    preserve = preservation;
    address = site;
    destination = target;
    direct = external;
    exitId = exit_id;
    patchSize = patch_size;
    originalSize = original_size;
    payloadSize = payload_size;
    destinationCapacity = static_cast<u32>(capacity);
    currentBytes = originalBytes + originalSize;
    patchedBytes = currentBytes + originalSize;
    destinationBytes = patchedBytes + patchSize;
    if (!memory.read(memory.context, address, originalBytes, originalSize)) {
        Clear();
        return false;
    }

    if (needs_destination) {
        trampoline = memory.allocate(memory.context, destinationCapacity, 16);
        if (trampoline == 0 || (trampoline & 3) != 0 || !Range_Valid(trampoline, destinationCapacity) ||
            (static_cast<u64>(trampoline) < static_cast<u64>(address) + originalSize &&
             static_cast<u64>(address) < static_cast<u64>(trampoline) + destinationCapacity)) {
            Clear();
            return false;
        }
    }

    return true;
}

bool Patch::Create_Hook(const Patch_Memory& provider, u32 site, u32 target, Patch_Preserve preservation) {
    return Begin(provider, site, Patch_Mode::Detour, preservation, 0, true, target) && Finish();
}

bool Patch::Create_Hook(const Patch_Memory& provider, const Signature& signature, const Memory_View& search, u32 target, Patch_Preserve preservation) {
    u32 site;
    return signature.Find_Unique(search, &site) && Create_Hook(provider, site, target, preservation);
}

bool Patch::Create_Hypercall(const Patch_Memory& provider, u32 site, u32 id, Patch_Mode placement, Patch_Preserve preservation) {
    if (id > 0xFFFFF || !Begin(provider, site, placement, preservation, 4)) {
        return false;
    }

    ModLoader::Bytes::Write_Be32(mode == Patch_Mode::Inline ? patchedBytes : destinationBytes, Encode_Hypercall(id));
    return Finish();
}

bool Patch::Create_Callback(const Patch_Memory& provider, u32 site, u32 entry_id, u32 exit_id) {
    if (entry_id == 0 || exit_id == 0 || entry_id > 0xFFFFF || exit_id > 0xFFFFF || entry_id == exit_id ||
        !Begin(provider, site, Patch_Mode::Detour, Patch_Preserve::Source, 4, false, 0, exit_id)) {
        return false;
    }
    ModLoader::Bytes::Write_Be32(destinationBytes, Encode_Hypercall(entry_id));
    return Finish();
}

bool Patch::Create_Assembly(const Patch_Memory& provider, u32 site, const char* source, Patch_Mode placement, Patch_Preserve preservation, Symbol_Resolver resolver, void* resolver_context) {
    if (created || source == nullptr) {
        return false;
    }

    Assembler assembled(resolver, resolver_context);
    if (!assembled.Assemble(source, site) || assembled.Origin() != site) {
        return false;
    }

    if (!Begin(provider, site, placement, preservation, assembled.Size())) {
        return false;
    }

    if (placement == Patch_Mode::Detour && (!assembled.Assemble(source, trampoline) ||
        assembled.Origin() != trampoline || assembled.Size() != payloadSize)) {
        Clear();
        return false;
    }

    memcpy(mode == Patch_Mode::Inline ? patchedBytes : destinationBytes, assembled.Data(), assembled.Size());
    return Finish();
}

bool Patch::Create_Bytes(const Patch_Memory& provider, u32 site, const u8* bytes, u32 size) {
    if (bytes == nullptr || !Begin(provider, site, Patch_Mode::Inline, Patch_Preserve::None, size)) {
        return false;
    }
    memcpy(patchedBytes, bytes, size);
    return Finish();
}

bool Patch::Prepare(const u8* original, u8* patched, u8* output, u32* size, u32* after) const {
    *size = 0;
    *after = 0;
    if (mode == Patch_Mode::Inline) {
        return true;
    }

    u32 words[Detail::gMaxRelocatedWords + 2];
    u32 delay = preserve == Patch_Preserve::Delay_Slot ? ModLoader::Bytes::Read_Be32(original + 4) : gNop;
    if (!Detail::Detour_Words(address, direct ? destination : trampoline, words, delay)) {
        return false;
    }

    ModLoader::Bytes::Write_Be32(patched, words, 2);
    if (trampoline == 0) {
        return true;
    }

    u32 count;
    if (preserve == Patch_Preserve::Source) {
        u32 instructions[3] = { ModLoader::Bytes::Read_Be32(original), ModLoader::Bytes::Read_Be32(original + 4), ModLoader::Bytes::Read_Be32(original + 8) };
        count = Detail::Relocate_Displaced(instructions, address, trampoline + payloadSize, words);
        if (count == 0) {
            return false;
        }
    }
    else {
        if (!Detail::Detour_Words(trampoline + payloadSize, address + 8, words)) {
            return false;
        }
        count = 2;
    }

    if (exitId != 0) {
        *after = trampoline + payloadSize + count * 4;
        words[count++] = Encode_Hypercall(exitId);
    }

    *size = payloadSize + count * 4;
    if (*size > destinationCapacity) {
        return false;
    }

    ModLoader::Bytes::Write_Be32(output + payloadSize, words, count);
    return true;
}

bool Patch::Finish() {
    const u8* payload = mode == Patch_Mode::Inline ? patchedBytes : destinationBytes;
    u32 origin = mode == Patch_Mode::Inline ? address : trampoline;
    if ((!direct && !Detail::Payload_Valid(payload, payloadSize, origin)) ||
        !Prepare(originalBytes, patchedBytes, destinationBytes, &destinationSize, &afterAddress) ||
        (mode == Patch_Mode::Detour && memcmp(originalBytes, patchedBytes, patchSize) == 0)) {
        Clear();
        return false;
    }
    created = true;
    return true;
}

u32 Patch::Installed_Size(const u8* bytes) const {
    if (memcmp(bytes, patchedBytes, patchSize) == 0) {
        return patchSize;
    }
    if (mode == Patch_Mode::Detour && memcmp(bytes, patchedBytes, sizeof(u32)) == 0) {
        return sizeof(u32);
    }
    return 0;
}

bool Patch::Write_Destination(const u8* bytes, u32 size) const {
    u8 current[128];
    for (u32 offset = 0; offset < size;) {
        u32 count = size - offset;
        if (count > sizeof(current)) {
            count = sizeof(current);
        }

        if (!memory.read(memory.context, trampoline + offset, current, count)) {
            return false;
        }

        if (memcmp(current, bytes + offset, count) != 0) {
            return memory.write(memory.context, trampoline, bytes, size);
        }

        offset += count;
    }
    return true;
}

bool Patch::Enable() {
    if (!created || !memory.read(memory.context, address, currentBytes, originalSize)) {
        return false;
    }

    if (!Entry_Valid(memory, address)) {
        Disable();
        return false;
    }

    u32 installed_size = Installed_Size(currentBytes);
    bool installed = installed_size == patchSize;
    enabled = installed;
    if (installed_size != 0 && !installed && preserve != Patch_Preserve::Delay_Slot) {
        Disable();
        return false;
    }
    memcpy(currentBytes, originalBytes, installed_size);

    bool changed = memcmp(currentBytes, originalBytes, originalSize) != 0;
    u8* next = nullptr;
    u8* next_patch = patchedBytes;
    u8* next_destination = destinationBytes;
    u32 next_size = destinationSize;
    u32 next_after = afterAddress;
    if (changed) {
        next = static_cast<u8*>(malloc(static_cast<size_t>(patchSize) + destinationCapacity));
        if (next == nullptr) {
            return false;
        }
        next_patch = next;
        next_destination = next + patchSize;
        memcpy(next_patch, patchedBytes, patchSize);
        memcpy(next_destination, destinationBytes, destinationSize);
        if (!Prepare(currentBytes, next_patch, next_destination, &next_size, &next_after)) {
            free(next);
            Disable();
            return false;
        }
    }

    bool success = Write_Destination(next_destination, next_size) && (installed || memory.write(memory.context, address, next_patch, patchSize));
    if (success) {
        memcpy(originalBytes, currentBytes, originalSize);
        if (changed) {
            memcpy(patchedBytes, next_patch, patchSize);
            memcpy(destinationBytes, next_destination, next_size);
        }
        destinationSize = next_size;
        afterAddress = next_after;
        enabled = true;
    }

    free(next);
    return success;
}

bool Patch::Disable() {
    if (!created) {
        return true;
    }

    if (!memory.read(memory.context, address, currentBytes, patchSize)) {
        return false;
    }

    u32 restore_size = Installed_Size(currentBytes);
    if (restore_size != 0 && !memory.write(memory.context, address, originalBytes, restore_size)) {
        return false;
    }

    enabled = false;
    return true;
}

bool Patch::Destroy() {
    if (!Disable()) {
        return false;
    }
    Clear();
    return true;
}

void Patch::Clear() {
    if (trampoline != 0) {
        memory.release(memory.context, trampoline);
    }

    free(originalBytes);
    originalBytes = nullptr;
    patchedBytes = nullptr;
    destinationBytes = nullptr;
    currentBytes = nullptr;
    memory = {};
    address = 0;
    destination = 0;
    trampoline = 0;
    afterAddress = 0;
    patchSize = 0;
    originalSize = 0;
    payloadSize = 0;
    destinationSize = 0;
    destinationCapacity = 0;
    exitId = 0;
    direct = false;
    created = false;
    enabled = false;
    mode = Patch_Mode::Inline;
    preserve = Patch_Preserve::None;
}

} // namespace N64
