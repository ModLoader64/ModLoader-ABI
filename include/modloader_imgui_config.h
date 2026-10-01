// Client and module layouts must match.
#pragma once

#define ImDrawIdx unsigned int
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

#define MODLOADER_IMGUI_LAYOUT \
    { sizeof(ImGuiIO),          sizeof(ImGuiStyle),          sizeof(ImVec2), \
      sizeof(ImVec4),           sizeof(ImDrawVert),          sizeof(ImDrawIdx), \
      sizeof(ImGuiViewport),    sizeof(ImGuiPayload),        sizeof(ImFontConfig), \
      sizeof(ImGuiListClipper), sizeof(ImGuiTableSortSpecs), sizeof(ImGuiTableColumnSortSpecs), \
      sizeof(ImGuiWindowClass), sizeof(ImTextureRef) \
    }
