#include "app_state.hpp"
#include "imgui.h"
#include "implot.h"
#include <cmath>
#include <algorithm>

extern AppState g_state;

// Maps a phase angle (radians) to a neon color
// 0°=cyan, 90°=magenta, 180°=yellow, 270°=lime
static ImU32 PhaseColor(double phase_rad, int alpha = 255) {
    float t = (float)(phase_rad / (M_PI * 2.0));
    if (t < 0.0f) t += 1.0f;
    t = fmodf(t, 1.0f);

    float r, g, b;
    // Smooth hue cycle: cyan → magenta → yellow → lime → cyan
    if (t < 0.25f) {
        float s = t / 0.25f;
        r = s;         g = 1.0f - s * 0.5f; b = 1.0f;
    } else if (t < 0.50f) {
        float s = (t - 0.25f) / 0.25f;
        r = 1.0f;      g = 0.5f - s * 0.5f; b = 1.0f - s;
    } else if (t < 0.75f) {
        float s = (t - 0.50f) / 0.25f;
        r = 1.0f;      g = s;               b = s * 0.8f;
    } else {
        float s = (t - 0.75f) / 0.25f;
        r = 1.0f - s;  g = 1.0f;            b = s;
    }
    return IM_COL32((int)(r*255), (int)(g*255), (int)(b*255), alpha);
}

static void DrawNeonHistogram(const std::vector<double>& probs,
                               const std::vector<double>& phases,
                               const std::vector<std::string>& labels)
{
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 canvas_pos  = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size = ImGui::GetContentRegionAvail();

    // Reserve space so ImGui knows layout height
    ImGui::InvisibleButton("##neon_hist", canvas_size);

    int n = (int)probs.size();
    if (n == 0) return;

    const float pad_l = 8.0f, pad_r = 8.0f;
    const float pad_t = 12.0f, pad_b = 36.0f; // bottom for labels
    float plot_w = canvas_size.x - pad_l - pad_r;
    float plot_h = canvas_size.y - pad_t - pad_b;
    if (plot_w <= 0 || plot_h <= 0) return;

    float origin_x = canvas_pos.x + pad_l;
    float origin_y = canvas_pos.y + pad_t + plot_h; // bottom of bars

    // Background
    draw->AddRectFilled(
        ImVec2(canvas_pos.x, canvas_pos.y),
        ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
        IM_COL32(4, 8, 18, 255), 6.0f);

    // Subtle grid lines
    int grid_lines = 4;
    for (int g = 1; g <= grid_lines; ++g) {
        float gy = origin_y - (plot_h * g / grid_lines);
        draw->AddLine(
            ImVec2(origin_x, gy),
            ImVec2(origin_x + plot_w, gy),
            IM_COL32(40, 80, 120, 60), 0.5f);
        // Grid label
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%%", (int)(25 * g));
        draw->AddText(ImVec2(origin_x + 2, gy - 10), IM_COL32(60, 120, 160, 180), buf);
    }

    // Baseline
    draw->AddLine(
        ImVec2(origin_x, origin_y),
        ImVec2(origin_x + plot_w, origin_y),
        IM_COL32(30, 100, 160, 120), 1.0f);

    float bar_gap   = 4.0f;
    float bar_total = plot_w / n;
    float bar_w     = bar_total - bar_gap;
    if (bar_w < 2.0f) bar_w = 2.0f;

    double max_prob = *std::max_element(probs.begin(), probs.end());
    if (max_prob < 1e-9) max_prob = 1.0;

    for (int i = 0; i < n; ++i) {
        if (probs[i] < 1e-5) continue;

        float bx     = origin_x + i * bar_total + bar_gap * 0.5f;
        float bar_h  = (float)(probs[i] / max_prob) * plot_h;
        float by_top = origin_y - bar_h;

        double phase = (i < (int)phases.size()) ? phases[i] : 0.0;
        ImU32 col_bright = PhaseColor(phase, 255);
        ImU32 col_mid    = PhaseColor(phase, 160);
        ImU32 col_dim    = PhaseColor(phase, 40);
        ImU32 col_glow   = PhaseColor(phase, 18);

        // Outer glow (wide, very transparent)
        float gw = bar_w * 0.6f;
        draw->AddRectFilled(
            ImVec2(bx - gw, by_top - 4.0f),
            ImVec2(bx + bar_w + gw, origin_y),
            col_glow, 3.0f);

        // Inner glow
        draw->AddRectFilled(
            ImVec2(bx - 2.0f, by_top - 2.0f),
            ImVec2(bx + bar_w + 2.0f, origin_y),
            col_dim, 2.0f);

        // Bar body (dark fill so neon edge stands out)
        draw->AddRectFilled(
            ImVec2(bx, by_top),
            ImVec2(bx + bar_w, origin_y),
            IM_COL32(8, 16, 32, 220), 2.0f);

        // Bright neon outline
        draw->AddRect(
            ImVec2(bx, by_top),
            ImVec2(bx + bar_w, origin_y),
            col_mid, 2.0f, 0, 1.0f);

        // Hot top cap line
        draw->AddLine(
            ImVec2(bx + 1.0f, by_top),
            ImVec2(bx + bar_w - 1.0f, by_top),
            col_bright, 2.0f);

        // Probability % label above bar
        char prob_buf[16];
        snprintf(prob_buf, sizeof(prob_buf), "%.0f%%", probs[i] * 100.0);
        float text_x = bx + bar_w * 0.5f - ImGui::CalcTextSize(prob_buf).x * 0.5f;
        draw->AddText(ImVec2(text_x, by_top - 14.0f), col_bright, prob_buf);

        // Phase label inside bar (if tall enough)
        if (bar_h > 28.0f) {
            char ph_buf[16];
            snprintf(ph_buf, sizeof(ph_buf), "%.0f°", phase * 180.0 / M_PI);
            float px = bx + bar_w * 0.5f - ImGui::CalcTextSize(ph_buf).x * 0.5f;
            draw->AddText(ImVec2(px, by_top + 4.0f), IM_COL32(255,255,255,140), ph_buf);
        }

        // Ket label below baseline
        if (i < (int)labels.size()) {
            float lx = bx + bar_w * 0.5f - ImGui::CalcTextSize(labels[i].c_str()).x * 0.5f;
            draw->AddText(ImVec2(lx, origin_y + 5.0f), IM_COL32(120, 200, 255, 200), labels[i].c_str());
        }
    }

    // Phase legend (bottom right)
    const char* legend = "Bar color = phase angle";
    ImVec2 leg_pos = ImVec2(
        canvas_pos.x + canvas_size.x - ImGui::CalcTextSize(legend).x - 6.0f,
        canvas_pos.y + canvas_size.y - 14.0f);
    draw->AddText(leg_pos, IM_COL32(60, 120, 160, 180), legend);
}

void RenderStatePanel() {
    ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "STATE VISUALIZER");

    if (ImGui::BeginTabBar("StateTabs")) {

        // --- TAB 1: AMPLITUDES ---
        if (ImGui::BeginTabItem("Amplitudes")) {
            auto probs = g_state.statevec.probabilities();
            int n = probs.size();

            if (ImGui::BeginTable("AmpsTable", 4,
                ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
                ImVec2(0, -1)))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Ket",       ImGuiTableColumnFlags_WidthFixed,   40.0f);
                ImGui::TableSetupColumn("Amplitude", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Prob %",    ImGuiTableColumnFlags_WidthFixed,   60.0f);
                ImGui::TableSetupColumn("Phase",     ImGuiTableColumnFlags_WidthFixed,   50.0f);
                ImGui::TableHeadersRow();

                for (int i = 0; i < n; ++i) {
                    if (probs[i] < 1e-4) continue;
                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "%s",
                        g_state.statevec.ket_label(i).c_str());

                    ImGui::TableNextColumn();
                    ImGui::Text("%.2f %+.2fi",
                        g_state.statevec.amplitudes[i].real,
                        g_state.statevec.amplitudes[i].imag);

                    ImGui::TableNextColumn();
                    ImGui::Text("%.1f%%", probs[i] * 100.0);

                    ImGui::TableNextColumn();
                    double phase_deg = g_state.statevec.amplitudes[i].phase() * 180.0 / M_PI;
                    ImU32 pcol = PhaseColor(g_state.statevec.amplitudes[i].phase());
                    ImGui::TextColored(
                        ImVec4(
                            ((pcol >> 0)  & 0xFF) / 255.0f,
                            ((pcol >> 8)  & 0xFF) / 255.0f,
                            ((pcol >> 16) & 0xFF) / 255.0f,
                            1.0f),
                        "%.0f°", phase_deg);
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        // --- TAB 2: HISTOGRAM ---
        if (ImGui::BeginTabItem("Histogram")) {
            auto probs = g_state.statevec.probabilities();
            int n = (int)probs.size();

            std::vector<double>      hist_probs;
            std::vector<double>      hist_phases;
            std::vector<std::string> hist_labels;

            // If shots have been run, use shot counts normalised; else use statevector probs
            if (g_state.shots_ready && !g_state.shot_outcomes.empty()) {
                double total_shots = 0;
                for (auto& [k, v] : g_state.shot_outcomes) total_shots += v;
                for (auto& [k, v] : g_state.shot_outcomes) {
                    int idx = (int)k;
                    hist_probs.push_back(v / total_shots);
                    hist_phases.push_back(idx < n ? g_state.statevec.amplitudes[idx].phase() : 0.0);
                    hist_labels.push_back(idx < n ? g_state.statevec.ket_label(idx) : std::to_string(idx));
                }
            } else {
                for (int i = 0; i < n; ++i) {
                    if (probs[i] < 1e-4) continue;
                    hist_probs.push_back(probs[i]);
                    hist_phases.push_back(g_state.statevec.amplitudes[i].phase());
                    hist_labels.push_back(g_state.statevec.ket_label(i));
                }
            }

            if (hist_probs.empty()) {
                ImGui::TextDisabled("No states to display.");
            } else {
                DrawNeonHistogram(hist_probs, hist_phases, hist_labels);
            }

            ImGui::EndTabItem();
        }

        // --- TAB 3: PAULI EXPECTATIONS ---
        if (ImGui::BeginTabItem("Expectation")) {
            if (ImGui::BeginTable("ExpTable", 3,
                ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
                ImVec2(0, -1)))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Qubit", ImGuiTableColumnFlags_WidthFixed,   60.0f);
                ImGui::TableSetupColumn("<X>",   ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("<Z>",   ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (int i = 0; i < g_state.n_qubits; ++i) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Q%d", i);
                    ImGui::TableNextColumn();
                    ImGui::Text("%+.3f", g_state.statevec.expect_X(i));
                    ImGui::TableNextColumn();
                    ImGui::Text("%+.3f", g_state.statevec.expect_Z(i));
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}