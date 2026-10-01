#pragma once

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum ModLoader_Rml_Event_Phase {
    MODLOADER_RML_EVENT_PHASE_NONE = 0,
    MODLOADER_RML_EVENT_PHASE_CAPTURE = 1u << 0,
    MODLOADER_RML_EVENT_PHASE_TARGET = 1u << 1,
    MODLOADER_RML_EVENT_PHASE_BUBBLE = 1u << 2,
} ModLoader_Rml_Event_Phase;

typedef enum ModLoader_Rml_Event_Button {
    MODLOADER_RML_EVENT_BUTTON_LEFT = 0,
    MODLOADER_RML_EVENT_BUTTON_RIGHT = 1,
    MODLOADER_RML_EVENT_BUTTON_MIDDLE = 2,
    MODLOADER_RML_EVENT_BUTTON_NONE = -1,
} ModLoader_Rml_Event_Button;

typedef struct ModLoader_Rml_Event {
    char type[32];
    uint32_t target;
    uint32_t current;
    uint32_t phase;
    uint32_t modifiers;
    float mouseX;
    float mouseY;
    float wheelX;
    float wheelY;
    int32_t button;
    uint32_t key;
    uint32_t parameterSize;
    uint32_t argumentCount;
} ModLoader_Rml_Event;

typedef enum ModLoader_Rml_Modifier_Flags {
    MODLOADER_RML_MODIFIER_CTRL = 1u << 0,
    MODLOADER_RML_MODIFIER_SHIFT = 1u << 1,
    MODLOADER_RML_MODIFIER_ALT = 1u << 2,
    MODLOADER_RML_MODIFIER_META = 1u << 3,
} ModLoader_Rml_Modifier_Flags;

typedef enum ModLoader_Rml_Propagation_Flags {
    MODLOADER_RML_STOP = 1u << 0,
    MODLOADER_RML_STOP_IMMEDIATE = 1u << 1, // also skips remaining listeners on this element
} ModLoader_Rml_Propagation_Flags;

typedef enum ModLoader_Rml_Element_Field {
    MODLOADER_RML_INNER_RML = 0,
    MODLOADER_RML_ATTRIBUTE = 1, // named; set without a value removes it
    MODLOADER_RML_PROPERTY = 2, // named; no value removes it
    MODLOADER_RML_VALUE = 3,
    MODLOADER_RML_CLASS = 4, // presence of value toggles membership; get returns "1" or ""
    MODLOADER_RML_TAG = 5, // get only
    MODLOADER_RML_ID = 6,
} ModLoader_Rml_Element_Field;

typedef enum ModLoader_Rml_Element_Relation {
    MODLOADER_RML_PARENT = 0,
    MODLOADER_RML_CHILD = 1,
    MODLOADER_RML_NEXT = 2,
    MODLOADER_RML_PREVIOUS = 3,
    MODLOADER_RML_DOCUMENT = 4,
} ModLoader_Rml_Element_Relation;

typedef enum ModLoader_Rml_Element_Action {
    MODLOADER_RML_FOCUS = 0,
    MODLOADER_RML_BLUR = 1,
    MODLOADER_RML_CLICK = 2,
    MODLOADER_RML_SCROLL_INTO_VIEW = 3, // argument 1 puts it at the top
    MODLOADER_RML_SHOW = 4, // documents: argument 0 hides, 1 shows, 2 shows modal
    MODLOADER_RML_CLOSE = 5, // documents
} ModLoader_Rml_Element_Action;

typedef struct ModLoader_Rml_Box {
    float x;
    float y;
    float width;
    float height;
    float scrollLeft;
    float scrollTop;
    float scrollWidth;
    float scrollHeight;
} ModLoader_Rml_Box;

typedef enum ModLoader_Rml_Model_Type {
    MODLOADER_RML_S32 = 0,
    MODLOADER_RML_U32 = 1,
    MODLOADER_RML_F32 = 2,
    MODLOADER_RML_F64 = 3,
    MODLOADER_RML_BOOL = 4, // one byte
    MODLOADER_RML_STRING = 5,
} ModLoader_Rml_Model_Type;

#if defined(__cplusplus)
}
#endif
