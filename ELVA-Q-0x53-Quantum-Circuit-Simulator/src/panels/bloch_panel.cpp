#include "app_state.hpp"
#include "imgui.h"
#include <cmath>

extern AppState g_state;
static float cam_yaw = 0.5f;
static float cam_pitch = 0.3f;
static float bloch_auto_speed = 25.0f; // degrees per second
struct Point3D { float x, y, z; };

void RenderBlochPanel() {
    ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "BLOCH SPHERE");
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 100);
    ImGui::SetNextItemWidth(100);
    ImGui::SliderInt("##BQ", &g_state.bloch_qubit, 0, g_state.n_qubits - 1, "Q%d");

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 avail = ImGui::GetContentRegionAvail();

    // PERFECT DYNAMIC CENTERING
    float size = std::min(avail.x, avail.y - 40.0f); // Reserve 40px for text at bottom
    ImVec2 center(pos.x + avail.x / 2.0f, pos.y + size / 2.0f);
    float R = size * 0.45f; // Radius is 45% of available space

    ImGui::InvisibleButton("##bloch_area", ImVec2(avail.x, size));
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        ImVec2 delta = ImGui::GetIO().MouseDelta;
        cam_yaw += delta.x * 0.01f;
        cam_pitch += delta.y * 0.01f;
        if(cam_pitch > M_PI/2.1f) cam_pitch = M_PI/2.1f;
        if(cam_pitch < -M_PI/2.1f) cam_pitch = -M_PI/2.1f;
    }

    // Auto-rotation — pauses while user is dragging
    if (!ImGui::IsItemActive()) {
        cam_yaw += bloch_auto_speed * (M_PI / 180.0f) * ImGui::GetIO().DeltaTime;
        if (cam_yaw > M_PI * 2.0f) cam_yaw -= M_PI * 2.0f;
    }

    auto Rotate3D = [&](float x, float y, float z) -> Point3D {
        float y1 = y * std::cos(cam_pitch) - z * std::sin(cam_pitch);
        float z1 = y * std::sin(cam_pitch) + z * std::cos(cam_pitch);
        float x2 = x * std::cos(cam_yaw) + z1 * std::sin(cam_yaw);
        float z2 = -x * std::sin(cam_yaw) + z1 * std::cos(cam_yaw);
        return {x2, y1, z2};
    };

    // Holographic Background
    draw->AddCircleFilled(center, R, IM_COL32(50, 0, 100, 15)); // Deep magenta core
    draw->AddCircle(center, R, IM_COL32(150, 50, 255, 90), 64, 2.0f); // Glowing violet outer rim

    auto DrawRing = [&](int axis) {
        ImVec2 prev_p; float prev_z = 0;
        for (int i = 0; i <= 64; ++i) {
            float angle = (i / 64.0f) * M_PI * 2.0f;
            float x=0, y=0, z=0;
            if (axis == 0) { x = std::cos(angle); y = std::sin(angle); }
            if (axis == 1) { x = std::cos(angle); z = std::sin(angle); }
            if (axis == 2) { y = std::cos(angle); z = std::sin(angle); }

            Point3D rot = Rotate3D(x, y, z);
            ImVec2 p2D(center.x + rot.x * R, center.y - rot.y * R);

            if (i > 0) {
                int alpha = ((prev_z + rot.z) / 2.0f) > 0 ? 140 : 25;
                draw->AddLine(prev_p, p2D, IM_COL32(0, 255, 255, alpha), 1.0f); // Cyan rings
            }
            prev_p = p2D; prev_z = rot.z;
        }
    };
    DrawRing(1); DrawRing(2);

    // Synthwave Axes (X=Pink, Y=Cyan, Z=Violet)
    Point3D ax_x = Rotate3D(1.15f, 0, 0); Point3D ax_y = Rotate3D(0, 1.15f, 0); Point3D ax_z = Rotate3D(0, 0, 1.15f);
    draw->AddLine(center, ImVec2(center.x + ax_x.x*R, center.y - ax_x.y*R), IM_COL32(255,40,165,180), 1.5f);
    draw->AddLine(center, ImVec2(center.x + ax_y.x*R, center.y - ax_y.y*R), IM_COL32(0,230,230,180), 1.5f);
    draw->AddLine(center, ImVec2(center.x + ax_z.x*R, center.y - ax_z.y*R), IM_COL32(150,50,255,180), 1.5f);

    draw->AddText(ImVec2(center.x + ax_x.x*R, center.y - ax_x.y*R), IM_COL32(255,100,180,255), "X");
    draw->AddText(ImVec2(center.x + ax_y.x*R, center.y - ax_y.y*R), IM_COL32(100,255,255,255), "Y");

    // Vector
    int q = g_state.bloch_qubit;
    double bx = g_state.statevec.expect_X(q), by = g_state.statevec.expect_Y(q), bz = g_state.statevec.expect_Z(q);

    Point3D state_rot = Rotate3D((float)bx, (float)by, (float)bz);
    ImVec2 tip(center.x + state_rot.x * R, center.y - state_rot.y * R);

    Point3D shadow_rot = Rotate3D((float)bx, 0, (float)bz);
    ImVec2 shadow(center.x + shadow_rot.x * R, center.y - shadow_rot.y * R);
    draw->AddLine(center, shadow, IM_COL32(255,180,25,80), 1.0f); // Gold shadow
    draw->AddLine(shadow, tip, IM_COL32(255,180,25,80), 1.0f);

    draw->AddLine(center, tip, IM_COL32(255, 180, 25, 255), 3.5f); // Bright Laser Gold Vector
    draw->AddCircleFilled(tip, 5.0f, IM_COL32(255, 255, 255, 255));
    ImGui::SetCursorPosY(pos.y + size + 5.0f);
    ImGui::Text("Expectation: X:%+.2f Y:%+.2f Z:%+.2f", bx, by, bz);
}