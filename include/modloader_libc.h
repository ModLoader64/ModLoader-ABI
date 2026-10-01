#pragma once

#include <stdint.h>

typedef struct ModLoader_Parsed_Float {
    uint64_t bits;
    int64_t length;
    int32_t error;
    uint32_t reserved;
} ModLoader_Parsed_Float;

#if defined(__cplusplus)
static_assert(sizeof(ModLoader_Parsed_Float) == 24);
static_assert(alignof(ModLoader_Parsed_Float) == 8);
#endif
