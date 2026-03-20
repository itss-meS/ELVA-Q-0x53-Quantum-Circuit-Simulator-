#include <GLFW/glfw3.h>

#include "imgui.h"
#include "implot.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "complex.hpp"
#include "statevector.hpp"
#include "gates.hpp"
#include "circuit.hpp"
#include "algorithms.hpp"
#include "app_state.hpp"

// Single translation unit includes
#include "panels/gate_palette.cpp"
#include "panels/circuit_panel.cpp"
#include "panels/state_panel.cpp"
#include "panels/bloch_panel.cpp"
#include "panels/algo_panel.cpp"
#include "panels/log_panel.cpp"
#include "app.cpp"

void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1440, 810, "ELVA-Q 0x53", nullptr, nullptr);
    if (!window) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // --- NEW: LOAD HIGH-QUALITY WINDOWS FONTS ---
    // Try Consolas (Monospaced tech font) first, then Segoe UI, then fallback.
    ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 18.0f);
    if (!font) font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 19.0f);
    if (!font) {
        io.Fonts->AddFontDefault();
        io.FontGlobalScale = 1.25f; // Only scale if using the small default pixel font
    }

    io.IniFilename = nullptr; // Lock layout

    SetupStyle();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    g_state.load_algorithm("Bell |Φ+>", quantum::algorithms::bell_phi_plus(), 2);
    g_state.log("System initialized. Welcome to ELVA-Q 0x53.", 0.0f, 0.85f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        if (display_w == 0 || display_h == 0) continue;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        RenderFrame(display_w, display_h);

        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.4f, 0.3f, 0.06f, 1.0f); // Pure Black Background
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}