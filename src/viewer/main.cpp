#include <iostream>

#include "app_povi.h"
#include <vector>

int main(void)
{
    std::cout << "start." << std::endl;
    poviApp a(1280, 800, "test-app");

    // Set up vertex data (and buffer(s)) and attribute pointers
    std::vector<float> vertices = {
        // Positions
        0.5f, -0.5f, 0.0f,  // Bottom Right
       -0.5f, -0.5f, 0.0f,  // Bottom Left
        0.0f,  0.5f, 0.0f  // Top 
    };
    // Colors
    //  1.0f, 0.0f, 0.0f,
    // 0.0f, 1.0f, 0.0f,
    // 0.0f, 0.0f, 1.0f 

    auto data_painter = std::make_shared<Painter>();

    data_painter->set_attribute("position", vertices.data(), vertices.size(), {3});
    data_painter->attach_shader("basic.vert");
    data_painter->attach_shader("basic.frag");
    data_painter->set_drawmode(GL_TRIANGLES);

    a.add_painter(std::move(data_painter), "triangle");

    a.run();
}
