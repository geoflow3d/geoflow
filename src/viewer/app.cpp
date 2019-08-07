// This file is part of Geoflow
// Copyright (C) 2018-2019  Ravi Peters, 3D geoinformation TU Delft

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "app.h"
#include <chrono>
#include <thread>
using namespace std::chrono_literals;

const std::string imgui_default_ini =
"[Window][GeoflowDockSpace]\n"
"Pos=0,0\n"
"Size=1280,800\n"
"Collapsed=0\n"
"\n"
"[Window][Debug##Default]\n"
"Pos=60,60\n"
"Size=400,400\n"
"Collapsed=0\n"
"\n"
"[Window][3D Viewer]\n"
"Pos=0,367\n"
"Size=997,433\n"
"Collapsed=0\n"
"DockId=0x00000003,0\n"
"\n"
"[Window][Flowchart]\n"
"Pos=0,19\n"
"Size=1280,346\n"
"Collapsed=0\n"
"DockId=0x00000001,0\n"
"\n"
"[Window][Painters]\n"
"Pos=999,367\n"
"Size=281,433\n"
"Collapsed=0\n"
"DockId=0x00000004,0\n"
"\n"
"[Docking][Data]\n"
"DockSpace     ID=0x72B5E78C Pos=0,19 Size=1280,781 Split=Y SelectedTab=0x26ED15F2\n"
"  DockNode    ID=0x00000001 Parent=0x72B5E78C SizeRef=1280,346 SelectedTab=0x84680795\n"
"  DockNode    ID=0x00000002 Parent=0x72B5E78C SizeRef=1280,433 Split=X SelectedTab=0x26ED15F2\n"
"    DockNode  ID=0x00000003 Parent=0x00000002 SizeRef=997,509 CentralNode=1 SelectedTab=0x26ED15F2\n"
"    DockNode  ID=0x00000004 Parent=0x00000002 SizeRef=281,509 SelectedTab=0x6A78F9B2\n";

App::App(int width, int height, std::string title)
	:width(width), height(height) {

    glfwSetErrorCallback(error_callback);
    
    if (!glfwInit())
        exit(EXIT_FAILURE);    // Set all the required options for GLFW

        // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
    
    window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwSetWindowUserPointer(window, this);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // set some OpenGL parameters
    glEnable(GL_PROGRAM_POINT_SIZE);
    // glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glfwSwapInterval(1);

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Load default layout if we can't find the default imgui.ini
    if (!std::ifstream("imgui.ini"))
        ImGui::LoadIniSettingsFromMemory(imgui_default_ini.c_str(), imgui_default_ini.size());
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigDockingWithShift = false;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them. 
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple. 
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // setup_callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetCharCallback(window, char_callback);
    glfwSetDropCallback(window, drop_callback);
}


void App::run(){
    
	on_initialise();

    // we'll try to make each iteration last at least this long, ie a limit on the FPS
    auto target_frame_duration = 16ms; // ~60FPS
    while (!glfwWindowShouldClose(window))
    { 
        auto start = std::chrono::high_resolution_clock::now();
        glfwWaitEvents(); // sleep until there is an event

        // redraw a couple of times to make sure ImGui is able to draw everything, see https://github.com/ocornut/imgui/issues/1206
        for(int i=0; i<=redraw_counter; i++){
            if (redraw_counter > 0)
                redraw_counter--;

            glfwMakeContextCurrent(window);
            // get framebuffer size, which could be different from the window size in case of eg a retina display  
            glfwGetFramebufferSize(window, &viewport_width, &viewport_height);
            glViewport(0, 0, viewport_width, viewport_height);
            // glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            // glClear(GL_COLOR_BUFFER_BIT);

            // process events:
            // if(!glfwGetWindowAttrib(window, GLFW_FOCUSED)) // don't do anything if window not in focus
            // else
            //     glfwPollEvents(); // don't sleep, eg needed for animations

            // Start the ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Central dockspace

            // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
            // because it would be confusing to have two docking targets within each others.
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_MenuBar;

            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus ;

            // When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
            // ImGui::SetNextWindowBgAlpha(0.0f);


            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            bool p_open;
            ImGui::Begin("GeoflowDockSpace", &p_open, window_flags);
            ImGui::PopStyleVar(3);

            // Dockspace
            ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));

            draw_menu_bar();
            ImGui::End();
            
            on_draw();

            if (show_demo_window)
            {
                ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
                ImGui::ShowDemoWindow(&show_demo_window);
            }

            // ImGui Rendering
            ImGui::Render();
            // int display_w, display_h;
            
            // glfwGetFramebufferSize(window, &display_w, &display_h);
            // glViewport(0, 0, display_w, display_h);
            
            // glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            
            glfwMakeContextCurrent(window);
            glfwSwapBuffers(window);
        }

        // sleep for the remainder of the rendering budget of this frame
        auto duration = std::chrono::high_resolution_clock::now()-start;
        if (duration < target_frame_duration){
            std::this_thread::sleep_for(target_frame_duration-duration);
        }
        
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}

void App::draw(){
    
}

void App::key_callback(
    GLFWwindow* window, int key, int scancode, int action, int mods
    ){
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

	// if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    //     glfwSetWindowShouldClose(window, GL_TRUE);

    void *data = glfwGetWindowUserPointer(window);  
    App *a = static_cast<App *>(data);
    a->on_key_press(key, action, mods);
}

void App::cursor_pos_callback(
    GLFWwindow* window, double xpos, double ypos
    ){
    if (ImGui::GetIO().WantCaptureMouse) return;
    
    void *data = glfwGetWindowUserPointer(window);  
    App *a = static_cast<App *>(data);
    // a->on_mouse_move(xpos, ypos);
}

void App::mouse_button_callback(
    GLFWwindow* window, int button, int action, int mods
    ){
    redraw_counter=4;
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }
    void *data = glfwGetWindowUserPointer(window);  
    App *a = static_cast<App *>(data);
    a->on_mouse_press(button, action, mods);
}

void App::scroll_callback(
    GLFWwindow* window, double xoffset, double yoffset
    ){
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    if (ImGui::GetIO().WantCaptureMouse) return;

    void *data = glfwGetWindowUserPointer(window);  
    App *a = static_cast<App *>(data);
    a->on_scroll(xoffset, yoffset);
}

void App::char_callback(
    GLFWwindow* window, unsigned int c
    ){
    ImGui_ImplGlfw_CharCallback(window, c);
}

void App::window_size_callback(
	GLFWwindow* window, int width, int height
	){

    void *data = glfwGetWindowUserPointer(window);
    App *a = static_cast<App *>(data);
    a->on_resize(width, height);
}

void App::error_callback(int error, const char* description) {
	std::cerr << error << " " << description;
}

void App::drop_callback(GLFWwindow* window, int count, const char** paths) {
    for (int i = 0;  i < count;  i++)
	    std::cout << paths[i] << "\n";

    void *data = glfwGetWindowUserPointer(window);
    App *a = static_cast<App *>(data);
    a->on_drop(count, paths);
}