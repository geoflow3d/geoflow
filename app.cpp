#include "app.h"

App::App(int width, int height, std::string title)
	:width(width), height(height) {

    glfwSetErrorCallback(error_callback);
    
    if (!glfwInit())
        exit(EXIT_FAILURE);    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwSetWindowUserPointer(window, this);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
}


void App::run(){
	on_initialise();
    while (!glfwWindowShouldClose(window))
    { 
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        on_draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);

}

void App::key_callback(
    GLFWwindow* window, int key, int scancode, int action, int mods
    ){

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    void *data = glfwGetWindowUserPointer(window);  
    App *a = static_cast<App *>(data);

    a->on_key_press(key, action, mods);
}

void App::cursor_pos_callback(
    GLFWwindow* window, double xpos, double ypos
    ){

    void *data = glfwGetWindowUserPointer(window);  
    App *a = static_cast<App *>(data);

    a->on_mouse_move(xpos, ypos);
}

void App::mouse_button_callback(
    GLFWwindow* window, int button, int action, int mods
    ){

    void *data = glfwGetWindowUserPointer(window);  
    App *a = static_cast<App *>(data);

    a->on_mouse_press(button, action, mods);
}

void App::scroll_callback(
    GLFWwindow* window, double xoffset, double yoffset
    ){

    void *data = glfwGetWindowUserPointer(window);  
    App *a = static_cast<App *>(data);

    a->on_mouse_move(xoffset, yoffset);
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