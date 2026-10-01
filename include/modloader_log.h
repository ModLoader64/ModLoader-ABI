#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum ModLoader_Log_Level {
    MODLOADER_LOG_ERROR = 0,
    MODLOADER_LOG_WARNING = 1,
    MODLOADER_LOG_INFO = 2,
    MODLOADER_LOG_DEBUG = 3,
} ModLoader_Log_Level;

#if defined(__cplusplus)
}
#endif
