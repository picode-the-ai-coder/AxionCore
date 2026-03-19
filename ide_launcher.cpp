#include "axion_ide.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>

using namespace axion;

// ====================================================================
// GLFW Error Callback
// ====================================================================
static void glfw_error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// ====================================================================
// Main IDE Application
// ====================================================================
int main(int argc, char* argv[])
{
    std::cout << "\n╔════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║           Axion IDE v2.0 - Smart Contract IDE             ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════╝\n" << std::endl;

    // ================================================================
    // Initialize GLFW
    // ================================================================
    glfwSetErrorCallback(glfw_error_callback);
    
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed!" << std::endl;
        return 1;
    }

    std::cout << "[INIT] GLFW initialized" << std::endl;

    // ================================================================
    // Create Window
    // ================================================================
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1400, 900, "Axion IDE v2.0", NULL, NULL);
    if (!window) {
        std::cerr << "Window creation failed!" << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    std::cout << "[INIT] Window created (1400x900)" << std::endl;

    // ================================================================
    // Setup ImGui
    // ================================================================
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(12, 12);
    style.FramePadding = ImVec2(8, 6);
    style.ItemSpacing = ImVec2(12, 8);
    style.FrameRounding = 4.0f;

    std::cout << "[INIT] ImGui initialized" << std::endl;

    // ================================================================
    // Setup ImGui Backends
    // ================================================================
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    std::cout << "[INIT] ImGui backends initialized" << std::endl;

    // ================================================================
    // Initialize AVM and IDE
    // ================================================================
    if (!init_advanced_avm()) {
        std::cerr << "AVM initialization failed!" << std::endl;
        glfwTerminate();
        return 1;
    }

    std::cout << "[INIT] Advanced AVM initialized" << std::endl;

    if (!init_axion_ide()) {
        std::cerr << "IDE initialization failed!" << std::endl;
        glfwTerminate();
        return 1;
    }

    AxionIDE* ide = get_ide();
    if (!ide->initialize("Axion IDE v2.0")) {
        std::cerr << "IDE setup failed!" << std::endl;
        glfwTerminate();
        return 1;
    }

    std::cout << "[INIT] Axion IDE initialized" << std::endl;
    std::cout << "\n✅ Ready! Use the IDE to deploy smart contracts.\n" << std::endl;

    // ================================================================
    // Main Loop
    // ================================================================
    while (!glfwWindowShouldClose(window) && !ide->should_close()) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render IDE
        ide->render_frame();

        // ImGui render
        ImGui::Render();

        // OpenGL rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // ================================================================
    // Cleanup
    // ================================================================
    std::cout << "\n[SHUTDOWN] Cleaning up..." << std::endl;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    shutdown_axion_ide();
    shutdown_advanced_avm();

    std::cout << "[SHUTDOWN] Complete\n" << std::endl;

    return 0;
}
