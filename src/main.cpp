#include <iostream>

#include "app_povi.h"
#include <array>

int main(void)
{
    std::cout << "start." << std::endl;
    poviApp a(1280, 800, "test-app");

    // Set up vertex data (and buffer(s)) and attribute pointers
    std::array<GLfloat,18> vertices = {
        // Positions         // Colors
        0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,  // Bottom Right
       -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,  // Bottom Left
        0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f   // Top 
    };

    auto data_painter = std::make_shared<Painter>();

   data_painter->set_data(vertices.data(), vertices.size(), {3,3});
   data_painter->attach_shader("basic.vert");
   data_painter->attach_shader("basic.frag");
   data_painter->set_drawmode(GL_TRIANGLES);

   a.add_painter(std::move(data_painter), "triangle");

    a.run();
}
