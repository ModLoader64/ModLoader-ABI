#pragma once

#include "modloader_events.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum ModLoader_Service_Event {
    MODLOADER_SERVICE_AVAILABLE = 0,
    MODLOADER_SERVICE_UNAVAILABLE = 1,
} ModLoader_Service_Event;

typedef enum ModLoader_Service_Error {
    MODLOADER_SERVICE_NOT_AVAILABLE = -1,
    MODLOADER_SERVICE_INVALID = -2,
    MODLOADER_SERVICE_BUSY = -3,
    MODLOADER_SERVICE_FAILED = -4,
} ModLoader_Service_Error;

typedef enum ModLoader_Service_Limit {
    MODLOADER_SERVICE_MAX_RECORD_SIZE = 1024 * 1024,
} ModLoader_Service_Limit;

typedef enum ModLoader_Service_Kind {
    MODLOADER_SERVICE_CALL = 0,
    MODLOADER_SERVICE_NOTIFY = 1,
} ModLoader_Service_Kind;

typedef struct ModLoader_Service_Record {
    ModLoader_Event_Header header;
    uint32_t kind;
    uint32_t handler;
    uint32_t operation;
    uint32_t reserved;
    uint64_t capacity;
    uint64_t size;
    // payload follows
} ModLoader_Service_Record;

#if defined(__cplusplus)
}
#endif

