#pragma once

#include <stdint.h>
#include "modloader_events.h"

typedef enum ModLoader_Texture_State {
    MODLOADER_TEXTURE_DISABLED = 0,
    MODLOADER_TEXTURE_PENDING = 1,
    MODLOADER_TEXTURE_READY = 2,
    MODLOADER_TEXTURE_UNSUPPORTED = 3,
    MODLOADER_TEXTURE_ERROR = 4,
} ModLoader_Texture_State;

typedef enum ModLoader_Texture_Flag {
    MODLOADER_TEXTURE_RELOAD = 1u,
} ModLoader_Texture_Flag;

typedef struct ModLoader_Texture_Event {
    ModLoader_Event_Header header;
    uint32_t source;
    uint32_t state;
    uint32_t enabled;
    uint32_t priority; // zero is highest
} ModLoader_Texture_Event;
