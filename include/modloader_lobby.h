#pragma once

#include "modloader_events.h"
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum ModLoader_Lobby_Event_Kind {
    MODLOADER_LOBBY_JOINED = 1,
    MODLOADER_LOBBY_MEMBER_JOINED = 2,
    MODLOADER_LOBBY_MEMBER_LEFT = 3,
    MODLOADER_LOBBY_MESSAGE = 4,
    MODLOADER_LOBBY_LEFT = 5,
    MODLOADER_LOBBY_SERVER_MESSAGE = 6,
} ModLoader_Lobby_Event_Kind;

typedef struct ModLoader_Lobby_Member {
    uint8_t id[16];
    char nickname[48]; // at most 32 bytes
} ModLoader_Lobby_Member;

typedef struct ModLoader_Lobby_Record {
    ModLoader_Event_Header header;
    uint32_t kind;
    uint32_t sealed;
    uint64_t size;
    ModLoader_Lobby_Member member;
    char topic[72]; // at most 64 bytes
} ModLoader_Lobby_Record;

typedef enum ModLoader_Server_Event_Kind {
    MODLOADER_SERVER_LOBBY_OPENED = 1,
    MODLOADER_SERVER_LOBBY_CLOSED = 2,
    MODLOADER_SERVER_MEMBER_JOINED = 3,
    MODLOADER_SERVER_MEMBER_LEFT = 4,
    MODLOADER_SERVER_MESSAGE = 5,
} ModLoader_Server_Event_Kind;

typedef struct ModLoader_Server_Record {
    ModLoader_Event_Header header;
    uint32_t kind;
    uint32_t lobby;
    uint64_t size;
    uint32_t sealed;
    uint32_t reserved;
    ModLoader_Lobby_Member member;
    char lobbyName[72]; // at most 64 bytes
    char topic[72]; // at most 64 bytes
    uint8_t publicKey[32]; // never sent to other clients
} ModLoader_Server_Record;

typedef enum ModLoader_Lobby_State {
    MODLOADER_LOBBY_STATUS_DISCONNECTED = 0,
    MODLOADER_LOBBY_STATUS_CONNECTING = 1,
    MODLOADER_LOBBY_STATUS_JOINED = 2,
    MODLOADER_LOBBY_STATUS_FAILED = 3,
} ModLoader_Lobby_State;

typedef struct ModLoader_Lobby_Status {
    uint32_t state;
    uint32_t memberCount; // excludes self
    ModLoader_Lobby_Member self;
    char lobby[72];
    uint8_t lobbyKey[32];
} ModLoader_Lobby_Status;

#if defined(__cplusplus)
}
#endif
