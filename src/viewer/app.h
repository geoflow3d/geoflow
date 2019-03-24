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

#include <iostream>
#include <string>
#include <fstream>
#include <cassert>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "gloo.h"

static int redraw_counter = 0;
class App
{
public:
  App  (int width, int height, std::string title);
  // ~app ();

   virtual void on_initialise(){};
   void run();
   void draw();
   virtual void draw_menu_bar(){};
   virtual void on_draw(){};
  
   virtual void on_resize(int new_width, int new_height){};  
   virtual void on_key_press(int key, int action, int mods){};
   virtual void on_scroll(double xoffset, double yoffset){};
   virtual void on_mouse_press(int button, int action, int mods){};
   virtual void on_mouse_move(double xpos, double ypos){};
   virtual void on_drop(int count, const char** paths){};

  int width, height;
  int viewport_width, viewport_height;
  bool show_demo_window = false;

protected:
  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
  static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
  static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
  static void window_size_callback(GLFWwindow* window, int new_width, int new_height);
  static void error_callback(int error, const char* description);
  static void char_callback(GLFWwindow*, unsigned int c);
  static void drop_callback(GLFWwindow* window, int count, const char** paths);

  GLFWwindow* window;
};
