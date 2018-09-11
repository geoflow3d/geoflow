#include "app_povi.h"

void poviApp::on_initialise(){

    model = glm::mat4(1.0f);

    glfwGetCursorPos(window, &last_mouse_pos.x, &last_mouse_pos.y);
}

void poviApp::center(float x, float y, float z) {
    center_point = glm::vec3(x,y,z);
}

void poviApp::add_painter(std::shared_ptr<Painter> painter, std::string name, bool visible) 
{
    painter->register_uniform(light_color);
    painter->register_uniform(light_direction);
    painters.push_back(std::make_tuple(painter, name, visible));
}

void poviApp::remove_painter(std::shared_ptr<Painter> painter) 
{
    for (auto t=painters.begin(); t!=painters.end(); ) {
        if (std::get<0>(*t) == painter) {
            painter->unregister_uniform(light_color);
            painter->unregister_uniform(light_direction);
            t = painters.erase(t);
        } else {
            t++;
        }
    }
}

void poviApp::on_resize(int new_width, int new_height) {
    width = new_width;
    height = new_height;
}

void poviApp::on_draw(){
    // Render
    update_view_matrix();
    update_projection_matrix();
    GLbitfield bits = 0;
    bits |= GL_COLOR_BUFFER_BIT;
    bits |= GL_DEPTH_BUFFER_BIT;
    bits |= GL_STENCIL_BUFFER_BIT;
    glClear(bits);
    for (auto &painter:painters){
        if (std::get<2>(painter))
            std::get<0>(painter)->render(model, view, projection);
    }
    if (drag != NO_DRAG)
        ch_painter.render(model, view, projection);

    if (drawthis_func)
        drawthis_func();

    // ImGui::Begin("View parameters");
    // xy_pos p0 = screen2view(last_mouse_pos);
    // ImGui::Text("Mouse pos [screen]: (%g, %g)", last_mouse_pos.x, last_mouse_pos.y);
    // ImGui::Text("Mouse pos [view]: (%g, %g)", p0.x, p0.y);
    ImGui::SliderFloat("Field of view", &fov, 1, 180);
    ImGui::SliderFloat("Clip near", &clip_near, 0.01, 100);
    ImGui::SliderFloat("Clip far", &clip_far, 1, 1000);
    // ImGui::SliderFloat("Camera position", &cam_pos, -200, -1);
    cam_pos->gui();
    light_color->gui();
    light_direction->gui();
    // ImGui::SliderFloat("Scale", &scale, 0.01, 100);
    // ImGui::End();

    ImGui::Begin("Painters");
    for(auto &painter:painters){
        auto p = std::get<0>(painter);
        ImGui::Checkbox(std::get<1>(painter).c_str(), &std::get<2>(painter));
        ImGui::Indent();
        ImGui::PushItemWidth(150);
        p->gui();
        ImGui::PopItemWidth();
        ImGui::Unindent();
    }
    ImGui::End();
}

void poviApp::center() {
    bbox.clear();
    for (auto &painter:painters){
        auto p_bbox = std::get<0>(painter)->get_bbox();
        if (!p_bbox.isEmpty())
            bbox.add(p_bbox);
    }
    translation = -bbox.center();
}

void poviApp::on_key_press(int key, int action, int mods) {
    if (action == GLFW_PRESS && key == GLFW_KEY_C) {
        center();
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_T) {
        rotation = glm::quat();
    }
}

void poviApp::on_mouse_press(int button, int action, int mods) {   
    
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        if(mods & GLFW_MOD_SHIFT)
            drag = TRANSLATE;
        else
            drag = ROTATE;

        glfwGetCursorPos(window, &drag_init_pos.x, &drag_init_pos.y);
    } else if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) {
        if(drag==TRANSLATE){
            translation += translation_ondrag;
            translation_ondrag = glm::vec3(0);
        }
        drag = NO_DRAG;
    }
}


void poviApp::on_mouse_move(double xpos, double ypos) {
    if (drag==TRANSLATE) {
        xy_pos delta = { xpos-drag_init_pos.x, ypos-drag_init_pos.y };

        double scale_ = -cam_pos->get_value() * 2* std::tan(glm::radians(fov/2.));
        // multiply with inverse view matrix and apply translation in world coordinates
        double radius = std::min(width, height);
        glm::vec4 t_screen = glm::vec4(scale_*delta.x/radius, scale_*-delta.y/radius, 0., 0.);
        translation_ondrag =  glm::vec3( glm::inverse(view) * t_screen );
        // update_view_matrix();
    } else if (drag==ROTATE) {
        xy_pos p0 = screen2view(last_mouse_pos);
        xy_pos p1 = screen2view({xpos, ypos});

        glm::quat q0 = arcball(p0);
        glm::quat q1 = arcball(p1);
        rotation =  q1 * q0 * rotation;
        // update_view_matrix();
    }
    last_mouse_pos.x = xpos;
    last_mouse_pos.y = ypos;
}
void poviApp::on_scroll(double xoffset, double yoffset){
    cam_pos->get_value() += yoffset/10;
}

void poviApp::update_view_matrix(){
    auto t = glm::mat4(1.0);
    t = glm::translate(t, glm::vec3(0.0f,0.0f,cam_pos->get_value()));
    t = t * glm::mat4_cast(rotation);
    t = glm::translate(t, translation);
    t = glm::translate(t, translation_ondrag);
    view = t;
}
void poviApp::update_projection_matrix(){
    projection = glm::perspective(glm::radians(fov), float(width)/float(height), clip_near, clip_far );       
}

inline xy_pos poviApp::screen2view(xy_pos p){
    double radius = std::min(width, height);
    return {(p.x-width/2.)/radius, ((height-p.y)-height/2.)/radius};
}