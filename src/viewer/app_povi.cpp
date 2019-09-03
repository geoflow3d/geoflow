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

#include "app_povi.h"

void poviApp::on_initialise(){

    model = glm::mat4(1.0f);

    glfwGetCursorPos(window, &last_mouse_pos.x, &last_mouse_pos.y);

    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    FramebufferName = 0;
    glGenFramebuffers(1, &FramebufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    
    // Now we need to create the texture which will contain the RGB output of our shader. This code is very classic :

    // The texture we're going to render to
    glGenTextures(1, &renderedTexture);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, renderedTexture);

    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, 1024, 768, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

    // Poor filtering. Needed !
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  
    
    // We also need a depth buffer. This is optional, depending on what you actually need to draw in your texture; but since we’re going to render Suzanne, we need depth-testing.

    // The depth buffer
    glGenRenderbuffers(1, &depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 768);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
    
    // Finally, we configure our framebuffer

    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);
    
    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
    // Something may have gone wrong during the process, depending on the capabilities of the GPU. This is how you check it :

    // Always check that our framebuffer is ok
    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: something wrong with the framebuffer setup\n";
        std::cerr << "Status code:" << status << "\n";
    }
}

void poviApp::center(float x, float y, float z) {
    center_point = glm::vec3(x,y,z);
}

void poviApp::add_painter(std::shared_ptr<Painter> painter, const  std::string* name, bool visible) 
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
//     width = new_width;
//     height = new_height;
}
void poviApp::draw_menu_bar() {
    if (ImGui::BeginMenuBar())
    {
        for (auto& render_object : render_objects) {
            render_object->menu();
        }
        // if (ImGui::BeginMenu("3D Viewer"))
        // {
            
        //     ImGui::EndMenu();
        // }
        if (ImGui::BeginMenu("Debug"))
        {
            if (ImGui::MenuItem("ImGui demo window", ""))
                show_demo_window=true;
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void poviApp::on_draw(){
    // Render painters
    // Render to framebuffer
    ImGui::Begin("3D Viewer");
    ImGui::BeginChild("3DChild", ImVec2(0,0), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking);
        
        if (ImGui::IsMouseReleased(1) && ImGui::IsWindowHovered()) {
            ImGui::OpenPopup("3DViewConfig");
        }

        auto size = ImGui::GetContentRegionAvail();
        width = size.x;
        height = size.y;


        if (ImGui::IsWindowHovered()) {
            auto& io = ImGui::GetIO();
            auto local_mouse_pos_x = io.MousePos.x - ImGui::GetWindowPos().x;
            auto local_mouse_pos_y = io.MousePos.y - ImGui::GetWindowPos().y;
            if (io.MouseWheel)
                on_scroll(io.MouseWheelH, io.MouseWheel);

            if (ImGui::IsMouseClicked(0)) {
                if(io.KeyCtrl)
                    drag = TRANSLATE;
                else
                    drag = ROTATE;
                drag_init_pos.x = local_mouse_pos_x;
                drag_init_pos.y = local_mouse_pos_y;
            } else if (ImGui::IsMouseReleased(0)) {
                if(drag==TRANSLATE){
                    translation += translation_ondrag;
                    translation_ondrag = glm::vec3(0);
                }
                drag = NO_DRAG;
            }
            on_mouse_move(local_mouse_pos_x, local_mouse_pos_y);
        }

        glBindTexture(GL_TEXTURE_2D, renderedTexture);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, size.x, size.y, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);

        glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
        glViewport(0,0,size.x,size.y); // Render on the whole framebuffer, complete from the lower left corner to the upper right

        update_view_matrix();
        update_projection_matrix();
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
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

        ImGui::Image((void*)(intptr_t)renderedTexture, size, ImVec2(0,1), ImVec2(1,0));

        if (ImGui::BeginPopup("3DViewConfig")) {
            if (ImGui::Button("Center (c)")) {
                center();
            }
            if (ImGui::Button("Reset rotation (t)")) {
                rotation = glm::quat();
            }
            ImGui::Separator();
            // ImGui::Begin("View parameters");
            // xy_pos p0 = screen2view(last_mouse_pos);
            // ImGui::Text("Mouse pos [screen]: (%g, %g)", last_mouse_pos.x, last_mouse_pos.y);
            // ImGui::Text("Mouse pos [view]: (%g, %g)", p0.x, p0.y);
            ImGui::SliderFloat("Field of view", &fov, 1, 180);
            ImGui::SliderFloat("Clip near", &clip_near, 0.01, 100);
            ImGui::SliderFloat("Clip far", &clip_far, 1, 1000);
            ImGui::SliderFloat("Zoom speed", &zoom_speed, 1, 1000);
            ImGui::ColorEdit4("Clear color", (float*)&clear_color);
            // ImGui::SliderFloat("Camera position", &cam_pos, -200, -1);
            cam_pos->gui();
            light_color->gui();
            light_direction->gui();
            // ImGui::SliderFloat("Scale", &scale, 0.01, 100);
            // ImGui::End();
            ImGui::EndPopup();
        }
    ImGui::EndChild();
    ImGui::End();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    for (auto& render_object : render_objects) {
        render_object->render();
    }


    ImGui::Begin("Painters");
    for(auto &painter:painters){
        auto p = std::get<0>(painter);
        ImGui::PushID(p.get());

        ImGui::Checkbox(std::get<1>(painter)->c_str(), &std::get<2>(painter));
        
        ImGui::SameLine();
        if (ImGui::Button("Center"))
            translation = -glm::make_vec3(p->get_bbox().center().data());
        ImGui::SameLine();
        // if (ImGui::Button("...")) 
        //     ImGui::OpenPopup("config");
        if (ImGui::CollapsingHeader("More")) {
            // p->short_gui();
            ImGui::Indent();
            ImGui::PushItemWidth(150);
            p->gui();
            ImGui::PopItemWidth();
            ImGui::Unindent();
        }

        ImGui::PopID();
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
    translation = -glm::make_vec3(bbox.center().data());
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
    
    // if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
    //     if(mods & GLFW_MOD_SHIFT)
    //         drag = TRANSLATE;
    //     else
    //         drag = ROTATE;

    //     glfwGetCursorPos(window, &drag_init_pos.x, &drag_init_pos.y);
    // } else if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) {
    //     if(drag==TRANSLATE){
    //         translation += translation_ondrag;
    //         translation_ondrag = glm::vec3(0);
    //     }
    //     drag = NO_DRAG;
    // }
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
    auto& io = ImGui::GetIO();
    float multiply = io.KeyCtrl ? zoom_speed : 1.0;
    cam_pos->get_value() += (yoffset/10) * multiply;
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