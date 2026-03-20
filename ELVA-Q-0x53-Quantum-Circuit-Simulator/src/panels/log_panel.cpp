#include"app_state.hpp"
#include "imgui.h"

extern AppState g_state;

void RenderLogPanel() {
    ImGui::BeginGroup();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImU32 status_col = g_state.circuit.ops.empty() ? IM_COL32(255, 200, 0, 255) : IM_COL32(0, 255, 0, 255);
    draw->AddCircleFilled(ImVec2(p.x + 10, p.y + 10), 5.0f, status_col);
    ImGui::SetCursorScreenPos(ImVec2(p.x + 25, p.y + 2));
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), " Statevector | Qubits: %d | Gates: %zu | Dim: %zu",
                       g_state.n_qubits, g_state.circuit.ops.size(), g_state.statevec.size);
    ImGui::EndGroup();

    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 100);
    if (ImGui::Button("Clear Log", ImVec2(100, 0))) {
        g_state.logs.clear();
    }

    ImGui::Separator();

    ImGui::BeginChild("LogScroll", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    for (const auto& log : g_state.logs) {
        ImGui::TextColored(ImVec4(log.r, log.g, log.b, 1.0f), "> %s", log.msg.c_str());
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
}