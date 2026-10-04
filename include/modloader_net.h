#pragma once

#include "modloader_events.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum ModLoader_Net_Transport {
    MODLOADER_NET_TCP = 0,
    MODLOADER_NET_UDP = 1,
} ModLoader_Net_Transport;

enum {
    MODLOADER_NET_MAX_DATAGRAM_SIZE = 65507,
    MODLOADER_LOBBY_MAX_UDP_MESSAGE = 60 * 1024,
};

typedef enum ModLoader_Net_Event_Kind {
    MODLOADER_NET_PACKET = 1,
    MODLOADER_NET_ACCEPTED = 2,
    MODLOADER_NET_CLOSED = 3,
} ModLoader_Net_Event_Kind;

typedef struct ModLoader_Net_Record {
    ModLoader_Event_Header header;
    uint32_t kind;
    uint32_t socket;
    uint32_t accepted;
    uint32_t address; // IPv4
    uint32_t port;
    uint32_t version;
    uint64_t size;
    char type[65];
    uint8_t reserved[3];
    uint32_t transport;
    // payload follows
} ModLoader_Net_Record;

#if defined(__cplusplus)
}
#endif
