#pragma once

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum ModLoader_Event {
    MODLOADER_EVENT_REFRESH = 0,
    MODLOADER_EVENT_RESET = 1,
    MODLOADER_EVENT_STATE_LOADED = 2,
    MODLOADER_EVENT_FRAME = 3,
    MODLOADER_EVENT_PAUSED = 0x40,
    MODLOADER_EVENT_PAUSE = 0x41,
    MODLOADER_EVENT_RESUME = 0x42,
    MODLOADER_EVENT_RESTART = 0x43,
    MODLOADER_EVENT_LOBBY = 0x80,
    MODLOADER_EVENT_SERVER = 0x81,
    MODLOADER_EVENT_UI = 0x82,
    MODLOADER_EVENT_IMAGE_RESIZED = 0x83,
    MODLOADER_EVENT_SETTING = 0x84,
    MODLOADER_EVENT_NET = 0x85,
    MODLOADER_EVENT_TEXTURE_SOURCE = 0x87,
    MODLOADER_EVENT_PLATFORM = 0x100,
} ModLoader_Event;

typedef enum ModLoader_Event_Phase {
    MODLOADER_EVENT_PHASE_PRE = 0,
    MODLOADER_EVENT_PHASE_NORMAL = 1,
    MODLOADER_EVENT_PHASE_POST = 2,
} ModLoader_Event_Phase;

typedef enum ModLoader_Clock {
    MODLOADER_CLOCK_MILLISECONDS = 0,
} ModLoader_Clock;

typedef struct ModLoader_Event_Header {
    uint64_t time; // emulated nanoseconds since poweron
    uint64_t milliseconds; // the client's monotonic clock
} ModLoader_Event_Header;

typedef enum ModLoader_Pause_Reason {
    MODLOADER_PAUSE_ASKED = 0,
    MODLOADER_PAUSE_BREAKPOINT = 1,
} ModLoader_Pause_Reason;

typedef struct ModLoader_Pause_Record {
    ModLoader_Event_Header header;
    uint32_t reason;
    uint32_t reserved;
} ModLoader_Pause_Record;

typedef struct ModLoader_Image_Resized_Record {
    ModLoader_Event_Header header;
    uint64_t size;
} ModLoader_Image_Resized_Record;

#if defined(__cplusplus)
}
#endif
