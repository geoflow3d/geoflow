#include <GLFW/glfw3.h>
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