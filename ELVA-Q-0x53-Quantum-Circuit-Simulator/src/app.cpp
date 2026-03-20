#include "app_state.hpp"
#include "imgui.h"

AppState g_state;

void RenderGatePalette();
void RenderCircuitPanel();
void RenderStatePanel();
void RenderBlochPanel();
void RenderAlgoPanel();
void RenderLogPanel();

// ─── Persistent split ratios (survive across frames) ─────────────────────────
static float s_col_left  = 0.22f;  // left column width ratio
static float s_col_right = 0.25f;  // right column width ratio
// mid = 1.0 - left - right - gaps (auto)

static float s_left_row  = 0.60f;  // Gates / Algos split (left col)
static float s_mid_row   = 0.70f;  // Circuit / Logs split (mid col)
static float s_right_row = 0.55f;  // Bloch / State split (right col)

// ─── Splitter helper ──────────────────────────────────────────────────────────
// Draws an invisible drag handle. Returns true while dragging.
// axis: 0 = vertical splitter (resizes width), 1 = horizontal splitter (resizes height)
// invert=true  → drag right/down SHRINKS the panel (used for right column splitter)
// invert=false → drag right/down GROWS  the panel (default)
static void Splitter(const char* id, bool vertical,
                     float* ratio, float ratio_min, float ratio_max,
                     float total, float thickness = 6.0f, bool invert = false)
{
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    float  w      = vertical ? thickness : total;
    float  h      = vertical ? total     : thickness;

    ImGui::SetCursorScreenPos(cursor);
    ImGui::InvisibleButton(id, ImVec2(w, h));

    bool   hovered = ImGui::IsItemHovered();
    bool   active  = ImGui::IsItemActive();

    // Change cursor to resize arrow
    if (hovered || active)
        ImGui::SetMouseCursor(vertical ? ImGuiMouseCursor_ResizeEW
                                       : ImGuiMouseCursor_ResizeNS);

    // Drag logic
    if (active) {
        ImVec2 delta = ImGui::GetIO().MouseDelta;
        float  drag  = vertical ? delta.x : delta.y;
        if (invert) drag = -drag;  // flip direction for right-side splitter
        *ratio += drag / total;
        if (*ratio < ratio_min) *ratio = ratio_min;
        if (*ratio > ratio_max) *ratio = ratio_max;
    }

    // Visual line — brighter when hovered/active
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImU32 col = active  ? IM_COL32(100, 220, 255, 200) :
                hovered ? IM_COL32( 60, 160, 220, 140) :
                          IM_COL32( 20,  80, 130,  80);

    if (vertical) {
        float mx = cursor.x + thickness * 0.5f;
        draw->AddLine(ImVec2(mx, cursor.y), ImVec2(mx, cursor.y + total), col, 1.5f);
    } else {
        float my = cursor.y + thickness * 0.5f;
        draw->AddLine(ImVec2(cursor.x, my), ImVec2(cursor.x + total, my), col, 1.5f);
    }
}

void SetupStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // ARCTIC ICE THEME
    colors[ImGuiCol_Text]                  = ImVec4(0.94f, 0.97f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.35f, 0.50f, 0.65f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.01f, 0.03f, 0.07f, 1.00f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.03f, 0.07f, 0.12f, 1.00f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.03f, 0.07f, 0.12f, 1.00f);
    colors[ImGuiCol_Border]                = ImVec4(0.05f, 0.28f, 0.50f, 0.60f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.03f, 0.07f, 0.12f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.04f, 0.14f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.05f, 0.20f, 0.35f, 1.00f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.01f, 0.03f, 0.07f, 1.00f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.02f, 0.08f, 0.16f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.01f, 0.03f, 0.07f, 1.00f);
    colors[ImGuiCol_MenuBarBg]             = ImVec4(0.02f, 0.05f, 0.10f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.01f, 0.03f, 0.07f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.05f, 0.28f, 0.55f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.06f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.40f, 0.91f, 0.98f, 1.00f);
    colors[ImGuiCol_CheckMark]             = ImVec4(0.40f, 0.91f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]            = ImVec4(0.05f, 0.57f, 0.91f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.40f, 0.91f, 0.98f, 1.00f);
    colors[ImGuiCol_Button]                = ImVec4(0.03f, 0.10f, 0.18f, 1.00f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.05f, 0.35f, 0.65f, 0.90f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.40f, 0.91f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                = ImVec4(0.03f, 0.12f, 0.22f, 1.00f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.05f, 0.35f, 0.65f, 0.90f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.40f, 0.91f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]             = ImVec4(0.05f, 0.20f, 0.38f, 0.80f);
    colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.05f, 0.57f, 0.91f, 1.00f);
    colors[ImGuiCol_SeparatorActive]       = ImVec4(0.40f, 0.91f, 0.98f, 1.00f);
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.05f, 0.28f, 0.55f, 0.50f);
    colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.05f, 0.57f, 0.91f, 0.80f);
    colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.40f, 0.91f, 0.98f, 1.00f);
    colors[ImGuiCol_Tab]                   = ImVec4(0.02f, 0.07f, 0.13f, 1.00f);
    colors[ImGuiCol_TabHovered]            = ImVec4(0.05f, 0.35f, 0.65f, 0.90f);
    colors[ImGuiCol_TabActive]             = ImVec4(0.04f, 0.18f, 0.32f, 1.00f);
    colors[ImGuiCol_TabUnfocused]          = ImVec4(0.02f, 0.05f, 0.10f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.03f, 0.12f, 0.22f, 1.00f);
    colors[ImGuiCol_PlotLines]             = ImVec4(0.40f, 0.91f, 0.98f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.66f, 0.91f, 0.98f, 1.00f);
    colors[ImGuiCol_PlotHistogram]         = ImVec4(0.05f, 0.57f, 0.91f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.40f, 0.91f, 0.98f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.05f, 0.35f, 0.65f, 0.50f);
    colors[ImGuiCol_NavHighlight]          = ImVec4(0.40f, 0.91f, 0.98f, 1.00f);

    style.WindowRounding    = 0.0f;
    style.ChildRounding     = 6.0f;
    style.FrameRounding     = 3.0f;
    style.GrabRounding      = 3.0f;
    style.PopupRounding     = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.TabRounding       = 4.0f;
    style.WindowBorderSize  = 0.0f;
    style.ChildBorderSize   = 1.0f;
    style.FrameBorderSize   = 1.0f;
    style.ItemSpacing       = ImVec2(8.0f, 8.0f);
}

void RenderFrame(int w, int h) {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)w, (float)h));
    ImGuiWindowFlags root_flags = ImGuiWindowFlags_NoDecoration
                                | ImGuiWindowFlags_NoMove
                                | ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
    ImGui::Begin("DashboardRoot", nullptr, root_flags);
    ImGui::PopStyleVar();

    const float splitter_w = 6.0f;  // drag handle thickness
    const float min_ratio  = 0.10f;
    const float max_ratio  = 0.70f;

    float total_h = (float)h - 16.0f; // account for window padding
    float total_w = (float)w - 16.0f;

    // ── Compute column widths from ratios ────────────────────────────────────
    // mid takes whatever is left after left + right + 2 splitters
    float splitter_total = splitter_w * 2.0f;
    float left_w  = (total_w - splitter_total) * s_col_left;
    float right_w = (total_w - splitter_total) * s_col_right;
    float mid_w   = (total_w - splitter_total) - left_w - right_w;

    // Clamp mid to a sane minimum
    if (mid_w < total_w * 0.20f) {
        mid_w = total_w * 0.20f;
    }

    // ── COLUMN 1: LEFT ───────────────────────────────────────────────────────
    ImGui::BeginChild("ColLeft", ImVec2(left_w, total_h), false);
    {
        float gates_h = total_h * s_left_row - splitter_w * 0.5f;
        float algos_h = total_h * (1.0f - s_left_row) - splitter_w * 0.5f;

        ImGui::BeginChild("Gates", ImVec2(left_w, gates_h), true, ImGuiWindowFlags_MenuBar);
        RenderGatePalette();
        ImGui::EndChild();

        // Horizontal splitter between Gates and Algos
        Splitter("##split_left_row", false, &s_left_row, min_ratio, max_ratio, left_w, splitter_w);

        ImGui::BeginChild("Algos", ImVec2(left_w, algos_h), true);
        RenderAlgoPanel();
        ImGui::EndChild();
    }
    ImGui::EndChild();

    // Vertical splitter between Left and Mid
    ImGui::SameLine();
    Splitter("##split_col_left", true, &s_col_left, min_ratio, max_ratio, total_h, splitter_w);
    ImGui::SameLine();

    // ── COLUMN 2: MIDDLE ─────────────────────────────────────────────────────
    ImGui::BeginChild("ColMid", ImVec2(mid_w, total_h), false);
    {
        float circuit_h = total_h * s_mid_row - splitter_w * 0.5f;
        float logs_h    = total_h * (1.0f - s_mid_row) - splitter_w * 0.5f;

        ImGui::BeginChild("Circuit", ImVec2(mid_w, circuit_h), true);
        RenderCircuitPanel();
        ImGui::EndChild();

        // Horizontal splitter between Circuit and Logs
        Splitter("##split_mid_row", false, &s_mid_row, min_ratio, max_ratio, mid_w, splitter_w);

        ImGui::BeginChild("Logs", ImVec2(mid_w, logs_h), true);
        RenderLogPanel();
        ImGui::EndChild();
    }
    ImGui::EndChild();

    // Vertical splitter between Mid and Right (invert=true: drag right=shrink, drag left=grow)
    ImGui::SameLine();
    Splitter("##split_col_right", true, &s_col_right, min_ratio, max_ratio, total_h, splitter_w, true);
    ImGui::SameLine();

    // ── COLUMN 3: RIGHT ──────────────────────────────────────────────────────
    ImGui::BeginChild("ColRight", ImVec2(right_w, total_h), false);
    {
        float bloch_h = total_h * s_right_row - splitter_w * 0.5f;
        float state_h = total_h * (1.0f - s_right_row) - splitter_w * 0.5f;

        ImGui::BeginChild("Bloch", ImVec2(right_w, bloch_h), true);
        RenderBlochPanel();
        ImGui::EndChild();

        // Horizontal splitter between Bloch and State
        Splitter("##split_right_row", false, &s_right_row, min_ratio, max_ratio, right_w, splitter_w);

        ImGui::BeginChild("State", ImVec2(right_w, state_h), true);
        RenderStatePanel();
        ImGui::EndChild();
    }
    ImGui::EndChild();

    ImGui::End();
}