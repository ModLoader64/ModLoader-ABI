#pragma once

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum ModLoader_Break_Access {
    MODLOADER_BREAK_EXECUTE = 1u << 0,
    MODLOADER_BREAK_READ = 1u << 1,
    MODLOADER_BREAK_WRITE = 1u << 2,
} ModLoader_Break_Access;

typedef struct ModLoader_Breakpoint {
    uint64_t start;
    uint64_t end;
    uint32_t access;
    uint32_t id;
    uint32_t processor;
    uint32_t reserved;
} ModLoader_Breakpoint;

typedef struct ModLoader_Break {
    uint32_t id; // 0 for a step
    uint32_t access;
    uint64_t address; // instruction address for execute; accessed address for read/write
    uint64_t value; // store value
    uint32_t size; // accessed bytes
    uint32_t processor;
} ModLoader_Break;

#if defined(__cplusplus)
}
#endif
