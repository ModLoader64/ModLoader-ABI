#pragma once

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum ModLoader_N64_Patch_Mode {
    MODLOADER_N64_PATCH_INLINE = 0,
    MODLOADER_N64_PATCH_DETOUR = 1,
} ModLoader_N64_Patch_Mode;

typedef enum ModLoader_N64_Patch_Preserve {
    MODLOADER_N64_PATCH_PRESERVE_NONE = 0,
    MODLOADER_N64_PATCH_PRESERVE_SOURCE = 1,
    MODLOADER_N64_PATCH_PRESERVE_DELAY_SLOT = 2,
} ModLoader_N64_Patch_Preserve;

typedef enum ModLoader_N64_Patch_Payload {
    MODLOADER_N64_PATCH_ASSEMBLY = 0,
    MODLOADER_N64_PATCH_HYPERCALL = 1,
    MODLOADER_N64_PATCH_DESTINATION = 2,
} ModLoader_N64_Patch_Payload;

typedef struct ModLoader_N64_Patch_Description {
    uint64_t address;
    uint64_t destination;
    uint64_t data;
    uint64_t dataSize;
    uint64_t handler;
    uint32_t mode;
    uint32_t payload;
    uint32_t preserve;
    uint32_t reserved;
} ModLoader_N64_Patch_Description;

#if defined(__cplusplus)
}
#endif
