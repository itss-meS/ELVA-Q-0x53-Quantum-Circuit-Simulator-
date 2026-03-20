#include "app_state.hpp"
#include "imgui.h"

extern AppState g_state;

void RenderGatePalette() {
    if (ImGui::BeginMenuBar()) {
        ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "GATE PALETTE");
        ImGui::EndMenuBar();
    }

    auto render_gate = [](const char* name, SelectedGate g, ImVec4 color) {
        bool sel = (g_state.selected_gate == g);
        if (sel) {
            ImGui::PushStyleColor(ImGuiCol_Button, color);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0,0,0,1)); // Black text on selected
        }

        // Button fills exact width of table column
        if (ImGui::Button(name, ImVec2(-FLT_MIN, 28))) g_state.selected_gate = g;

        if (sel) ImGui::PopStyleColor(2);
    };

    ImGui::TextDisabled("Single Qubit (Pauli & Hadamard)");
    if (ImGui::BeginTable("SingleQubit", 4, ImGuiTableFlags_SizingStretchSame)) {
        ImVec4 col_pink = ImVec4(1.0f, 0.15f, 0.65f, 0.8f); // Paulis
        ImVec4 col_cyan = ImVec4(0.0f, 0.90f, 0.90f, 0.8f); // Phase/Superposition

        ImGui::TableNextRow();
        ImGui::TableNextColumn(); render_gate("H", SelectedGate::H, col_cyan);
        ImGui::TableNextColumn(); render_gate("X", SelectedGate::X, col_pink);
        ImGui::TableNextColumn(); render_gate("Y", SelectedGate::Y, col_pink);
        ImGui::TableNextColumn(); render_gate("Z", SelectedGate::Z, col_pink);

        ImGui::TableNextRow();
        ImGui::TableNextColumn(); render_gate("S", SelectedGate::S, col_cyan);
        ImGui::TableNextColumn(); render_gate("T", SelectedGate::T, col_cyan);
        ImGui::EndTable();
    }

    ImGui::Spacing(); ImGui::TextDisabled("Parametric Rotations");
    if (ImGui::BeginTable("Rotations", 3, ImGuiTableFlags_SizingStretchSame)) {
        ImVec4 col_purple = ImVec4(0.6f, 0.2f, 1.0f, 0.8f);
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); render_gate("Rx", SelectedGate::Rx, col_purple);
        ImGui::TableNextColumn(); render_gate("Ry", SelectedGate::Ry, col_purple);
        ImGui::TableNextColumn(); render_gate("Rz", SelectedGate::Rz, col_purple);
        ImGui::EndTable();
    }

    ImGui::PushItemWidth(-FLT_MIN);
    if (g_state.selected_gate == SelectedGate::Rx) ImGui::SliderAngle("##rx", &g_state.rx_angle, -360, 360, "Angle X: %.0f deg");
    if (g_state.selected_gate == SelectedGate::Ry) ImGui::SliderAngle("##ry", &g_state.ry_angle, -360, 360, "Angle Y: %.0f deg");
    if (g_state.selected_gate == SelectedGate::Rz) ImGui::SliderAngle("##rz", &g_state.rz_angle, -360, 360, "Angle Z: %.0f deg");
    ImGui::PopItemWidth();

    ImGui::Spacing(); ImGui::TextDisabled("Entanglement & Measurement");
    if (ImGui::BeginTable("Multi", 2, ImGuiTableFlags_SizingStretchSame)) {
        ImVec4 col_cyan = ImVec4(0.0f, 0.90f, 0.90f, 0.8f);
        ImVec4 col_red = ImVec4(1.0f, 0.70f, 0.10f, 0.8f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn(); render_gate("CNOT", SelectedGate::CNOT, col_cyan);
        ImGui::TableNextColumn(); render_gate("SWAP", SelectedGate::SWAP, col_cyan);
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); render_gate("Toffoli", SelectedGate::Toffoli, col_cyan);
        ImGui::TableNextColumn(); render_gate("Measure", SelectedGate::Measure, col_red);
        ImGui::EndTable();
    }

    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

    ImGui::TextDisabled("Qubit Targeting:");
    ImGui::PushItemWidth(-FLT_MIN);
    ImGui::SliderInt("##TQ", &g_state.selected_qubit_a, 0, g_state.n_qubits - 1, "Target Qubit: %d");
    if (g_state.selected_gate == SelectedGate::CNOT || g_state.selected_gate == SelectedGate::SWAP || g_state.selected_gate == SelectedGate::Toffoli) {
        ImGui::SliderInt("##CQ1", &g_state.selected_qubit_b, 0, g_state.n_qubits - 1, "Control 1: %d");
    }
    if (g_state.selected_gate == SelectedGate::Toffoli) {
        ImGui::SliderInt("##CQ2", &g_state.selected_qubit_c, 0, g_state.n_qubits - 1, "Control 2: %d");
    }
    ImGui::PopItemWidth();

    ImGui::Dummy(ImVec2(0.0f, ImGui::GetContentRegionAvail().y - 70.0f)); // Push buttons to bottom

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.6f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.8f, 0.4f, 1.0f));
    if (ImGui::Button("+ ADD GATE", ImVec2(-FLT_MIN, 30))) {
        int q0 = g_state.selected_qubit_a, q1 = g_state.selected_qubit_b, q2 = g_state.selected_qubit_c;
        switch(g_state.selected_gate) {
            case SelectedGate::H: g_state.circuit.h(q0); break;
            case SelectedGate::X: g_state.circuit.x(q0); break;
            case SelectedGate::Y: g_state.circuit.y(q0); break;
            case SelectedGate::Z: g_state.circuit.z(q0); break;
            case SelectedGate::S: g_state.circuit.s(q0); break;
            case SelectedGate::T: g_state.circuit.t(q0); break;
            case SelectedGate::Rx: g_state.circuit.rx(q0, g_state.rx_angle); break;
            case SelectedGate::Ry: g_state.circuit.ry(q0, g_state.ry_angle); break;
            case SelectedGate::Rz: g_state.circuit.rz(q0, g_state.rz_angle); break;
            case SelectedGate::CNOT: g_state.circuit.cnot(q1, q0); break;
            case SelectedGate::SWAP: g_state.circuit.swap(q0, q1); break;
            case SelectedGate::Toffoli: g_state.circuit.toffoli(q2, q1, q0); break;
            case SelectedGate::Measure: g_state.circuit.measure(q0); break;
            default: break;
        }
        g_state.run_circuit();
    }
    ImGui::PopStyleColor(2);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    if (ImGui::Button("CLEAR CIRCUIT", ImVec2(-FLT_MIN, 30))) {
        g_state.circuit.clear();
        g_state.run_circuit();
        g_state.log("Circuit cleared", 1.0f, 0.3f, 0.3f);
    }
    ImGui::PopStyleColor(2);
}