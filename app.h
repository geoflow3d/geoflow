#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

class app
{
public:
  app  (int width, int height, std::string title);
  // ~app ();

  virtual void on_initialize() {};
  void run();
  virtual void on_draw() {};

  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void window_size_callback(GLFWwindow* window, int new_width, int new_height);
  static void error_callback(int error, const char* description);
  
  virtual void on_resize(int new_width, int new_height) {};  
  virtual void on_key_press(int key, int action, int mods) {};
  // virtual void on_mouse_wheel() {};
  // virtual void on_mouse_move() {};

// protected:
  GLFWwindow* window;
  int width;
  int height;

};
