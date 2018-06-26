#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/geometric.hpp>

#include "app.h"
#include "shader.h"

class poviApp: public App {
public:
poviApp(int width, int height, std::string title):App(width, height, title){}

protected:
void on_initialise();
void on_resize(int new_width, int new_height);
void on_draw();
void on_key_press(int key, int action, int mods){};
void on_mouse_wheel(double xoffset, double yoffset);
void on_mouse_move(double xpos, double ypos);
void on_mouse_press(int button, int action, int mods);
GLuint VBO, VAO;
Shader shader;

glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;

typedef struct {double x,y;} xy_pos;

bool is_mouse_drag = 0;
xy_pos drag_init_pos;

double radius;

double fov = 5;
double clip_near = 2;
double clip_far = 100;

double cam_pos = -12;
double scale = 0.6;
glm::vec3 translation;
glm::vec3 translation_ondrag;

void update_projection_matrix();
void update_view_matrix();

inline xy_pos screen2view(xy_pos p){
    return {(p.x-width/2.)/radius, ((height-p.y)-height/2.)/radius};
}
inline void update_radius(){
    radius = std::min(width, height);
}


};
