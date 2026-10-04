// Client and module layouts must match.
#pragma once

#include <stdint.h>

enum class ModLoader_ImGui_Font_Kind : uint32_t {
    Default = 0,
    Vector = 1,
    Bitmap = 2,
};

#define ImDrawIdx unsigned int
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

#define MODLOADER_IMGUI_LAYOUT \
    { sizeof(ImGuiIO),          sizeof(ImGuiStyle),          sizeof(ImVec2), \
      sizeof(ImVec4),           sizeof(ImDrawVert),          sizeof(ImDrawIdx), \
      sizeof(ImGuiViewport),    sizeof(ImGuiPayload),        sizeof(ImFontConfig), \
      sizeof(ImGuiListClipper), sizeof(ImGuiTableSortSpecs), sizeof(ImGuiTableColumnSortSpecs), \
      sizeof(ImGuiWindowClass), sizeof(ImTextureRef) \
    }
