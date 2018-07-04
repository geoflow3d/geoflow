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

    // std::unique_ptr<Buffer> data_buffer(new Buffer);
    auto data_buffer = std::make_unique<Buffer>();
    data_buffer->set_data(vertices.data(), vertices.size());
    data_buffer->add_field(3);
    data_buffer->add_field(3);

    // Shader data_shader;
    auto data_shader = std::make_unique<Shader>();
    data_shader->attach("basic.vert");
    data_shader->attach("basic.frag");
    
    data_painter->set_buffer(std::move(data_buffer));
    data_painter->set_program(std::move(data_shader));
    data_painter->set_drawmode(GL_TRIANGLES);

    a.add_painter(std::move(data_painter));

    a.run();
}
