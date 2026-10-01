#pragma once

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum ModLoader_Window_Mode {
    MODLOADER_WINDOW_HOST = 0,
    MODLOADER_WINDOW_ADAPTER = 1,
} ModLoader_Window_Mode;

typedef enum ModLoader_Frame_Kind {
    MODLOADER_FRAME_CPU = 0,
    MODLOADER_FRAME_GPU_SHARED = 1,
} ModLoader_Frame_Kind;

typedef enum ModLoader_Handle_Type {
    MODLOADER_HANDLE_NONE = 0,
    MODLOADER_HANDLE_OPAQUE_WIN32 = 1, // Vulkan NT handle
    MODLOADER_HANDLE_OPAQUE_FD = 2, // Vulkan POSIX descriptor
    MODLOADER_HANDLE_D3D12 = 3, // NT resource/fence handles; COMMON state
} ModLoader_Handle_Type;

typedef struct ModLoader_Gpu_Image {
    uint8_t deviceUuid[16]; // D3D12: adapter LUID
    uint64_t memoryHandle;
    uint64_t memorySize;
    uint32_t handleType;
    uint32_t imageId; // never reused in a session
    uint32_t vkFormat; // R8G8B8A8_UNORM or R16G16B16A16_UNORM
    uint32_t reserved;
    uint64_t readySemaphoreHandle; // shared timeline across frames
    uint64_t readyValue;
    uint64_t releaseSemaphoreHandle; // shared timeline across frames
    uint64_t releaseValue;
} ModLoader_Gpu_Image;

typedef enum ModLoader_Pixel_Format {
    MODLOADER_PIXELS_RGBA8 = 0,
    MODLOADER_PIXELS_RGBA16 = 1,
} ModLoader_Pixel_Format;

typedef struct ModLoader_Frame {
    uint32_t kind;
    uint32_t width;
    uint32_t height;
    uint32_t displayAspectNumerator;
    uint32_t displayAspectDenominator;
    uint32_t sourceWidth; // native output size before upscaling; 0: width and height
    uint32_t sourceHeight;
    uint32_t pitch;
    uint32_t format;
    const uint8_t* pixels;
    ModLoader_Gpu_Image gpu;
} ModLoader_Frame;

#if defined(__cplusplus)
}
#endif
