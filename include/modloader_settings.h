#pragma once

#include "modloader_events.h"
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum ModLoader_Setting_Type {
    MODLOADER_SETTING_BOOL = 0,
    MODLOADER_SETTING_INT = 1,
    MODLOADER_SETTING_FLOAT = 2,
    MODLOADER_SETTING_CHOICE = 3,
    MODLOADER_SETTING_TEXT = 4,
    MODLOADER_SETTING_BINDING = 5,
} ModLoader_Setting_Type;

typedef enum ModLoader_Setting_Flags {
    MODLOADER_SETTING_RESTART = 1u << 0,
    MODLOADER_SETTING_HIDDEN = 1u << 1,
    MODLOADER_SETTING_SPEED_LIMIT = 1u << 2,
    MODLOADER_SETTING_DISABLED = 1u << 3,
} ModLoader_Setting_Flags;

typedef struct ModLoader_Setting {
    const char* key;
    const char* label;
    const char* page;
    const char* help;
    const char* choices;
    const char* value;
    uint32_t type;
    uint32_t flags;
    double minimum;
    double maximum;
} ModLoader_Setting;

typedef struct ModLoader_Setting_Record {
    ModLoader_Event_Header header;
    char owner[64];
    char key[128];
} ModLoader_Setting_Record;

typedef struct ModLoader_Module_Setting {
    uint64_t key;
    uint64_t label;
    uint64_t help;
    uint64_t choices;
    uint64_t value; // default value
    uint32_t type;
    uint32_t flags;
    double minimum;
    double maximum;
} ModLoader_Module_Setting;

#if defined(__cplusplus)
}
#endif
