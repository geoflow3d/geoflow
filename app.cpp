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

	on_initialise();
}

void App::on_initialise(){

    
    // shader.attach("basic.vert").attach("basic.frag");
    // shader.link();

    // Set up vertex data (and buffer(s)) and attribute pointers
    GLfloat vertices[] = {
        // Positions         // Colors
        0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,  // Bottom Right
       -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,  // Bottom Left
        0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f   // Top 
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); // Unbind VAO

    // Shader shader;
    shader.init();
    shader.attach("basic.vert");
    shader.attach("basic.frag");
    shader.link();

    std::cout << "prog." << shader.get() << std::endl;
}

void App::on_resize(int new_width, int new_height) {
    width = new_width;
    height = new_height;
}

void App::on_draw(){
    // Render
    
    // Draw the triangle
    shader.activate();
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

void App::run(){
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