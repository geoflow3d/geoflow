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

class poviApp: public App {
public:
poviApp(int width, int height, std::string title):App(width, height, title){}
std::weak_ptr<Painter> add_painter(std::shared_ptr<Painter> painter, std::string name, bool visible=true);
void remove_painter(std::weak_ptr<Painter> painter); 
void draw_that(void (*func)()) { drawthis_func = func; };
void center(float, float, float z=0);

protected:
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
std::vector< std::tuple<std::shared_ptr<Painter>,std::string,bool> > painters;
void (*drawthis_func)();

glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;

mouse_drag drag = NO_DRAG;
xy_pos drag_init_pos;
xy_pos last_mouse_pos;

float fov = 60;
float clip_near = 0.01;
float clip_far = 100;

Painter ch_painter;
std::array<GLfloat,8> crosshair_lines = {
		-1,  0,
			1,  0,
			0, -1,
			0,  1
};

float cam_pos = -2;
float scale = 0.05;
glm::vec3 translation, center_point;
glm::vec3 translation_ondrag;
glm::quat rotation;
glm::quat rotation_ondrag;

void update_projection_matrix();
void update_view_matrix();
inline xy_pos screen2view(xy_pos p);

};
