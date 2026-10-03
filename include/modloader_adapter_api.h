#pragma once

#include "modloader_debug.h"
#include "modloader_events.h"
#include "modloader_log.h"
#include "modloader_platform.h"
#include "modloader_settings.h"
#include "modloader_video.h"
#include "modloader_texture.h"
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define MODLOADER_ADAPTER_ABI_VERSION 1u

typedef struct ModLoader_Host_Callbacks {
    void* host;
    void (*event)(void* host, uint32_t event, uint64_t time, const void* data, uint64_t size);
    int32_t (*hypercall)(void* host, uint32_t processor, uint32_t hypercall_id, void* state, uint64_t size);
    void (*log)(void* host, uint32_t level, const char* message);
    void (*presentFrame)(void* host, const ModLoader_Frame* frame);
    void (*pushAudio)(void* host, const int16_t* samples, uint32_t frame_count, uint32_t sample_rate);
    int32_t (*pollInput)(void* host, uint32_t port, void* out_state, uint64_t size);
    void (*setRumble)(void* host, uint32_t port, uint32_t strength);
    void (*breakpoint)(void* host, const ModLoader_Break* hit, void* state, uint64_t size);
    void (*outputSize)(void* host, uint32_t* out_width, uint32_t* out_height);
} ModLoader_Host_Callbacks;

typedef struct ModLoader_Adapter_Config {
    const uint8_t* imageData;
    uint64_t imageSize;
    const char* contentPath;
    const char* dataDirectory;
    uint32_t windowMode;
    uint8_t gpuUuid[16];
    const char* saveDirectory;
} ModLoader_Adapter_Config;

typedef struct ModLoader_Platform_Adapter {
    uint32_t abiVersion;
    uint32_t structSize;
    const char* platformIdentifier;
    const char* adapterIdentifier;
    int32_t (*create)(const ModLoader_Adapter_Config* config, const ModLoader_Host_Callbacks* callbacks, void** out_adapter);
    void (*destroy)(void* adapter);
    int32_t (*run)(void* adapter);
    void (*requestQuit)(void* adapter);
    uint32_t (*spaceCount)(void* adapter);
    int32_t (*spaceDescribe)(void* adapter, uint32_t space_index, ModLoader_Space_Descriptor* out_descriptor);
    uint32_t (*processorCount)(void* adapter);
    int32_t (*processorDescribe)(void* adapter, uint32_t processor, ModLoader_Processor_Descriptor* out_descriptor);
    int32_t (*invalidateCode)(void* adapter, uint32_t space_index, uint64_t address, uint64_t size);
    int32_t (*translateAddress)(void* adapter, uint32_t processor, uint64_t address, uint32_t* out_space_index, uint64_t* out_offset);
    int32_t (*saveState)(void* adapter, uint8_t* destination, uint64_t* inout_size);
    int32_t (*loadState)(void* adapter, const uint8_t* source, uint64_t size);
    int32_t (*setBreakpoints)(void* adapter, const ModLoader_Breakpoint* breakpoints, uint32_t count, uint32_t stepProcessor);
    int32_t (*resizeImage)(void* adapter, uint64_t size);
    uint32_t (*settingCount)(void* adapter);
    int32_t (*settingDescribe)(void* adapter, uint32_t index, ModLoader_Setting* out_setting);
    int32_t (*settingSet)(void* adapter, const char* key, const char* value);
    uint64_t (*peekMemory)(void* adapter, uint32_t processor, uint64_t address, uint8_t* buffer, uint64_t size);
    uint64_t (*pokeMemory)(void* adapter, uint32_t processor, uint64_t address, const uint8_t* data, uint64_t size);
    int32_t (*reset)(void* adapter);
    int64_t (*platformCall)(void* adapter, uint32_t operation, const void* request, uint64_t requestSize, void* response, uint64_t responseCapacity);
    int32_t (*setTextureSources)(void* adapter, const char* const* paths, uint32_t count, uint32_t flags);
} ModLoader_Platform_Adapter;

typedef const ModLoader_Platform_Adapter* (*ModLoader_Get_Platform_Adapter_Function)(void);

#if defined(__cplusplus)
}
#endif
