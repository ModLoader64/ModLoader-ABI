#pragma once

#include "modloader_events.h"
#include "modloader_n64_adapter.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum ModLoader_N64_Query {
    MODLOADER_N64_QUERY_IMAGE = 0,
} ModLoader_N64_Query;

typedef struct ModLoader_N64_Image {
    char sha1[48];
    char title[32];
    char gameCode[8];
    uint32_t version;
    uint64_t size;
} ModLoader_N64_Image;

typedef enum ModLoader_N64_Clock {
    MODLOADER_N64_CLOCK_VI = 1,
    MODLOADER_N64_CLOCK_TASK = 2,
    MODLOADER_N64_CLOCK_FRAME = 3,
} ModLoader_N64_Clock;

// MODLOADER_EVENT_REFRESH
typedef struct ModLoader_N64_Vi_Record {
    ModLoader_Event_Header header;
    uint64_t viCount;
    float viTime; // emulated seconds since the previous one
    uint32_t status;
    uint32_t origin; // KSEG0 address; 0 when blank
    uint32_t width;
} ModLoader_N64_Vi_Record;

typedef struct ModLoader_N64_Task_Record {
    ModLoader_Event_Header header;
    uint64_t taskCount;
    float taskTime; // emulated seconds since the previous one
    uint32_t reserved;
} ModLoader_N64_Task_Record;

typedef struct ModLoader_N64_Frame_Record {
    ModLoader_Event_Header header;
    uint64_t frameCount;
    float frameTime; // emulated seconds since the previous one
    uint32_t origin; // KSEG0 address
    uint32_t width;
    uint32_t reserved;
} ModLoader_N64_Frame_Record;

#if defined(__cplusplus)
}
#endif
