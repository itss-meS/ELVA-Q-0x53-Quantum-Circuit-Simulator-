#include "algorithms.hpp"
#include "app_state.hpp"
#include "imgui.h"

extern AppState g_state;

void RenderAlgoPanel() {
    ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.2f, 1.0f), "ALGORITHMS");
    ImGui::Separator();

    auto algo_btn = [](const char* name, const Circuit& c, int q) {
        if (ImGui::Button(name, ImVec2(-1, 0))) {
            g_state.load_algorithm(name, c, q);
        }
    };

    ImGui::Text("Bell States");
    algo_btn("|Φ+>", quantum::algorithms::bell_phi_plus(), 2);
    algo_btn("|Ψ+>", quantum::algorithms::bell_psi_plus(), 2);

    ImGui::Spacing(); ImGui::Text("Entanglement");
    algo_btn("GHZ(3)", quantum::algorithms::ghz(3), 3);
    algo_btn("GHZ(4)", quantum::algorithms::ghz(4), 4);

    ImGui::Spacing(); ImGui::Text("Transforms");
    algo_btn("QFT(3)", quantum::algorithms::qft(3), 3);
    algo_btn("QFT(4)", quantum::algorithms::qft(4), 4);

    ImGui::Spacing(); ImGui::Text("Search");
    algo_btn("Grover 2Q", quantum::algorithms::grover_2q(), 2);
    algo_btn("Deutsch-Jozsa", quantum::algorithms::deutsch_jozsa_constant(3), 3);
    algo_btn("Bernstein-V", quantum::algorithms::bernstein_vazirani(), 4);

    ImGui::Spacing(); ImGui::Text("Comm.");
    algo_btn("Teleportation", quantum::algorithms::teleportation(), 3);

    ImGui::Spacing(); ImGui::Text("Utility");
    algo_btn("QRNG(4)", quantum::algorithms::qrng(4), 4);

    ImGui::Dummy(ImVec2(0, ImGui::GetContentRegionAvail().y - 30));
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##Shots", &g_state.shot_count, 10, 10000, "Shots: %d");
}