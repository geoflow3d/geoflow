#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

class App
{
public:
  App  (int width, int height, std::string title);
  // ~app ();

  virtual void on_initialize() {};
  void run();
  virtual void on_draw() {};
  
  virtual void on_resize(int new_width, int new_height) {};  
  virtual void on_key_press(int key, int action, int mods) {};
  virtual void on_mouse_wheel(double xoffset, double yoffset) {};
  virtual void on_mouse_press(int button, int action, int mods) {};
  virtual void on_mouse_move(double xpos, double ypos) {};

  int width;
  int height;

protected:
  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
  static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
  static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
  static void window_size_callback(GLFWwindow* window, int new_width, int new_height);
  static void error_callback(int error, const char* description);

  GLFWwindow* window;

};
