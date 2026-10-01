#pragma once

#include "modloader_events.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum ModLoader_N64_Event {
    MODLOADER_EVENT_N64_RSP_TASK = MODLOADER_EVENT_PLATFORM,
} ModLoader_N64_Event;

typedef enum ModLoader_N64_Processor {
    MODLOADER_N64_PROCESSOR_VR4300 = 0,
} ModLoader_N64_Processor;

#define MODLOADER_N64_VR4300_STATE_FORMAT "n64.vr4300.v1"

// Spaces: "rdram", "rom", "sp_mem"

typedef struct ModLoader_N64_Cpu_State {
    uint64_t gpr[32]; // writes to gpr[0] are ignored
    uint64_t hi;
    uint64_t lo;
    uint64_t pc; // unchanged hypercall PC resumes at pc + 4
    uint64_t fpr[32]; // FR=0: f(2n + 1) is the high half of fpr[2n]
    uint32_t fcr31;
    // these are read-only
    uint32_t status; 
    uint32_t cause;
    uint32_t count;
} ModLoader_N64_Cpu_State;

typedef struct ModLoader_N64_Vi {
    uint32_t status;
    uint32_t origin; // physical RDRAM address
    uint32_t width; // pixels per line
    uint32_t reserved;
} ModLoader_N64_Vi;

typedef enum ModLoader_N64_Input_Shift {
    MODLOADER_N64_STICK_X_SHIFT = 16,
    MODLOADER_N64_STICK_Y_SHIFT = 24,
} ModLoader_N64_Input_Shift;

// bindings: "controller<port>.<control>"; a/b/z/start/l/r, and c_/dpad_/stick_ + up/down/left/right

#if defined(__cplusplus)
}
#endif
