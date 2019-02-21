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
