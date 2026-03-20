#include "app_state.hpp"
#include "imgui.h"

extern AppState g_state;

void RenderCircuitPanel() {
    // Header controls
    ImGui::PushItemWidth(100);
    int q_temp = g_state.n_qubits;
    if (ImGui::SliderInt("##Qubits", &q_temp, 1, 10, "Qubits: %d")) g_state.reset(q_temp);
    ImGui::PopItemWidth();

    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 210);
    if (ImGui::Button("Run Shots", ImVec2(100, 0))) g_state.run_shots();
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.9f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    if (ImGui::Button("SIMULATE", ImVec2(100, 0))) g_state.run_circuit();
    ImGui::PopStyleColor(3);

    ImGui::Separator();

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size = ImGui::GetContentRegionAvail();

    float y_step = 50.0f;
    float x_step = 50.0f;
    float start_y = pos.y + 35.0f;
    float start_x = pos.x + 40.0f;

    // PERFECT MATH BACKGROUND GRID (Dots align perfectly with wire Y-coordinates)
    draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(11, 9, 16, 255));
    for (float x = start_x; x < pos.x + size.x; x += x_step) {
        for (float y = start_y; y < pos.y + size.y; y += y_step) {
            draw->AddCircleFilled(ImVec2(x, y), 1.5f, IM_COL32(150, 100, 255, 15)); // Violet stardust dots
        }
    }

    ImVec2 m = ImGui::GetMousePos();

    // Wires
    for (int i = 0; i < g_state.n_qubits; ++i) {
        float y = start_y + i * y_step;
        bool hovered = (m.y >= y - 15 && m.y <= y + 15 && m.x >= pos.x && m.x <= pos.x + size.x);

        ImU32 text_col = hovered ? IM_COL32(0, 255, 255, 255) : IM_COL32(120, 100, 160, 255);
        ImU32 line_col = hovered ? IM_COL32(0, 255, 255, 180) : IM_COL32(70, 50, 100, 255); // Violet wires

        draw->AddText(ImVec2(pos.x + 5, y - 8), text_col, ("Q" + std::to_string(i)).c_str());
        draw->AddLine(ImVec2(start_x, y), ImVec2(pos.x + size.x, y), line_col, hovered ? 2.5f : 1.5f);
    }

    // Gates
    float cur_x = start_x + x_step;
    for (const auto& op : g_state.circuit.ops) {

        // Exact Color Categorization
        ImU32 col = IM_COL32(0, 204, 255, 255); // Cyan (H, S, T)
        if (op.type == GateType::X || op.type == GateType::Y || op.type == GateType::Z) col = IM_COL32(0, 255, 120, 255); // Green (Paulis)
        if (op.type == GateType::Rx || op.type == GateType::Ry || op.type == GateType::Rz) col = IM_COL32(200, 50, 255, 255); // Purple (Rotations)
        if (op.type == GateType::Measure) col = IM_COL32(255, 60, 60, 255); // Red (Measure)
        if (op.type == GateType::SWAP) col = IM_COL32(255, 200, 0, 255); // Yellow (SWAP)

        if (op.qubits.size() == 1) {
            float y = start_y + op.qubits[0] * y_step;
            bool hovered = (m.x >= cur_x - 16 && m.x <= cur_x + 16 && m.y >= y - 16 && m.y <= y + 16);

            if (hovered) {
                ImGui::SetTooltip("%s Gate", op.name.c_str());
                draw->AddRectFilled(ImVec2(cur_x - 18, y - 18), ImVec2(cur_x + 18, y + 18), IM_COL32(40, 60, 80, 255), 4.0f);
            } else {
                draw->AddRectFilled(ImVec2(cur_x - 16, y - 16), ImVec2(cur_x + 16, y + 16), IM_COL32(15, 20, 25, 255), 4.0f);
            }
            draw->AddRect(ImVec2(cur_x - 16, y - 16), ImVec2(cur_x + 16, y + 16), col, 4.0f, 0, 1.5f);

            // Perfect Text Centering
            ImVec2 tsize = ImGui::CalcTextSize(op.name.c_str());
            draw->AddText(ImVec2(cur_x - (tsize.x/2.0f), y - (tsize.y/2.0f)), col, op.name.c_str());
        }
        else if (op.qubits.size() == 2 && op.type == GateType::CNOT) {
            float y1 = start_y + op.qubits[0] * y_step;
            float y2 = start_y + op.qubits[1] * y_step;
            draw->AddLine(ImVec2(cur_x, y1), ImVec2(cur_x, y2), col, 2.0f);
            draw->AddCircleFilled(ImVec2(cur_x, y1), 5.0f, col);
            draw->AddCircle(ImVec2(cur_x, y2), 10.0f, col, 0, 2.0f);
            draw->AddLine(ImVec2(cur_x, y2 - 10), ImVec2(cur_x, y2 + 10), col, 2.0f);
            draw->AddLine(ImVec2(cur_x - 10, y2), ImVec2(cur_x + 10, y2), col, 2.0f); // Plus sign
        }
        else if (op.qubits.size() == 2 && op.type == GateType::SWAP) {
            float y1 = start_y + op.qubits[0] * y_step;
            float y2 = start_y + op.qubits[1] * y_step;
            draw->AddLine(ImVec2(cur_x, y1), ImVec2(cur_x, y2), col, 1.5f);
            draw->AddText(ImVec2(cur_x - 5, y1 - 8), col, "X");
            draw->AddText(ImVec2(cur_x - 5, y2 - 8), col, "X");
        }
        cur_x += x_step;
    }
}