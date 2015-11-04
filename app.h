#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cassert>

#include "shader.h"

class App
{
public:
  App  (int width, int height, std::string title);
  // ~app ();

   void on_initialise() ;
   void run();
   void on_draw();
  
   void on_resize(int new_width, int new_height);  
   void on_key_press(int key, int action, int mods){};
   void on_mouse_wheel(double xoffset, double yoffset){};
   void on_mouse_press(int button, int action, int mods){};
   void on_mouse_move(double xpos, double ypos){};

  int width;
  int height;
  GLuint VBO, VAO;
  GLuint mProgram;
  GLint  mStatus;
  GLint  mLength;

protected:
  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
  static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
  static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
  static void window_size_callback(GLFWwindow* window, int new_width, int new_height);
  static void error_callback(int error, const char* description);

  GLFWwindow* window;
  Shader shader;

};
