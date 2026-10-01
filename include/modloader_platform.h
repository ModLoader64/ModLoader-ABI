#pragma once

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum ModLoader_Space_Codec {
    MODLOADER_CODEC_LITTLE_ENDIAN = 0,
    MODLOADER_CODEC_BIG_ENDIAN = 1,
    MODLOADER_CODEC_BIG_ENDIAN_WORD_SWAPPED = 2,
} ModLoader_Space_Codec;

typedef enum ModLoader_Space_Flags {
    MODLOADER_SPACE_MAPPED = 1u << 0,
    MODLOADER_SPACE_WRITABLE = 1u << 1,
} ModLoader_Space_Flags;

typedef struct ModLoader_Space_Descriptor {
    const char* name;
    uint64_t size; // page aligned
    uint64_t usableSize; // bytes the host may touch
    uint64_t reportedSize; // what the game is told it has, 0 when not applicable
    uint32_t codec;
    uint32_t flags;
    uint8_t* hostBase;
} ModLoader_Space_Descriptor;

typedef struct ModLoader_Processor_Descriptor {
    const char* name;
    const char* stateFormat;
    uint64_t stateSize;
} ModLoader_Processor_Descriptor;

typedef struct ModLoader_Module_Space {
    char name[64];
    uint64_t size;
    uint64_t usableSize;
    uint64_t reportedSize;
    uint32_t codec;
    uint32_t flags;
} ModLoader_Module_Space;

typedef struct ModLoader_Module_Processor {
    char name[64];
    char stateFormat[64];
    uint64_t stateSize;
} ModLoader_Module_Processor;

typedef enum ModLoader_Session_Flags {
    MODLOADER_SESSION_GAME = 1u << 0,
    MODLOADER_SESSION_HOSTING = 1u << 1,
} ModLoader_Session_Flags;

typedef struct ModLoader_Platform_Description {
    uint32_t abiVersion;
    uint32_t session;
    uint32_t spaceCount;
    uint32_t processorCount;
    char platform[64];
} ModLoader_Platform_Description;

#if defined(__cplusplus)
}
#endif
