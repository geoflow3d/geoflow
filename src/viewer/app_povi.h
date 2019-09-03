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

#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <array>
#include <vector>
#include <memory>
#include <tuple>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
// #include <glm/gtx/quaternion.hpp>

#include "app.h"

enum mouse_drag{
    NO_DRAG,
    TRANSLATE,
    ROTATE
};

typedef struct {double x,y;} xy_pos;

inline glm::quat arcball(xy_pos p){
	double h,h2 = p.x*p.x+p.y*p.y;
	if (h2 > 1.){
		h = glm::sqrt(h2);
		return glm::quat(0., p.x/h, p.y/h, 0.);
	} else
		return glm::quat(0., p.x, p.y, glm::sqrt(1.-h2));
}

class RenderObject {
	public:
	virtual void render() = 0;
	virtual void menu(){};
};

class poviApp: public std::enable_shared_from_this<poviApp>, public App {
public:
poviApp(int width, int height, std::string title):App(width, height, title) {
	light_direction = std::make_shared<Uniform3f>("u_light_direction", glm::vec3(0.5,-1.0,-1.0));
	light_color = std::make_shared<Uniform4f>("u_light_color");
	cam_pos = std::make_shared<Uniform1f>("u_cam_pos", -15);
};
void add_painter(std::shared_ptr<Painter> painter, const std::string* name, bool visible=true);
void remove_painter(std::shared_ptr<Painter> painter); 
void draw_that(RenderObject* o) { render_objects.push_back(o); };
void center(float, float, float z=0);
std::shared_ptr<poviApp> get_ptr() {
	return shared_from_this();
};

protected:
void draw_menu_bar();
void on_initialise();
void on_resize(int new_width, int new_height);
void on_draw();
void on_key_press(int key, int action, int mods);
void on_scroll(double xoffset, double yoffset);
void on_mouse_move(double xpos, double ypos);
void on_mouse_press(int button, int action, int mods);

// Shader shader;
// Buffer buffer;
private:
GLuint FramebufferName, renderedTexture, depthrenderbuffer;
ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

std::vector< std::tuple<std::shared_ptr<Painter>,const std::string*,bool> > painters;
std::vector<RenderObject*> render_objects;

glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;

mouse_drag drag = NO_DRAG;
xy_pos drag_init_pos;
xy_pos last_mouse_pos;

geoflow::Box bbox;

float fov = 30;
float clip_near = 1;
float clip_far = 10000;
float zoom_speed = 200;
std::shared_ptr<Uniform1f> cam_pos;
std::shared_ptr<Uniform3f> light_direction;
std::shared_ptr<Uniform4f> light_color;

hudPainter ch_painter;

glm::vec3 translation = glm::vec3(), center_point = glm::vec3();
glm::vec3 translation_ondrag = glm::vec3();
glm::quat rotation = glm::quat();
glm::quat rotation_ondrag = glm::quat();

void update_projection_matrix();
void update_view_matrix();
inline xy_pos screen2view(xy_pos p);

void center();

};
