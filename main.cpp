#include "app.h"

class povi_app: public app {
public:

povi_app(int width, int height, std::string title):app(width, height, title){}

void on_initialize(){
    
}

void on_resize(int new_width, int new_height) {
    width = new_width;
    height = new_height;
}

void on_draw(){
    float ratio;
    ratio = width / (float) height;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef((float) glfwGetTime() * 50.f, 0.f, 0.f, 1.f);
    glBegin(GL_LINE_LOOP);
    glColor3f(1.f, 0.f, 0.f);
    glVertex3f(-0.6f, -0.4f, 0.f);
    glColor3f(0.f, 1.f, 0.f);
    glVertex3f(0.6f, -0.4f, 0.f);
    glColor3f(0.f, 0.f, 1.f);
    glVertex3f(0.f, 0.6f, 0.f);
    glEnd();
}


void on_key_press(int key, int action, int mods) {   
}

};

int main(void)
{
    povi_app a = povi_app(1024,768, "test-app");
    a.run();
}
