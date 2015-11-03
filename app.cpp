#include "app.h"

app::app(int width, int height, std::string title)
	:width(width), height(height) {

    glfwSetErrorCallback(error_callback);
    
    if (!glfwInit())
        exit(EXIT_FAILURE);

    window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwSetWindowUserPointer(window, this);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
	on_initialize();
}

void app::run(){
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

void app::key_callback(
    GLFWwindow* window, int key, int scancode, int action, int mods
    ){

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    void *data = glfwGetWindowUserPointer(window);  
    app *a = static_cast<app *>(data);

    a->on_key_press(key, action, mods);
}

void app::cursor_pos_callback(
    GLFWwindow* window, double xpos, double ypos
    ){

    void *data = glfwGetWindowUserPointer(window);  
    app *a = static_cast<app *>(data);

    a->on_mouse_move(xpos, ypos);
}

void app::mouse_button_callback(
    GLFWwindow* window, int button, int action, int mods
    ){

    void *data = glfwGetWindowUserPointer(window);  
    app *a = static_cast<app *>(data);

    a->on_mouse_press(button, action, mods);
}

void app::scroll_callback(
    GLFWwindow* window, double xoffset, double yoffset
    ){

    void *data = glfwGetWindowUserPointer(window);  
    app *a = static_cast<app *>(data);

    a->on_mouse_move(xoffset, yoffset);
}

void app::window_size_callback(
	GLFWwindow* window, int width, int height
	){

    void *data = glfwGetWindowUserPointer(window);  
    app *a = static_cast<app *>(data);

    a->on_resize(width, height);
}

void app::error_callback(int error, const char* description) {
	std::cerr << error << " " << description;
}