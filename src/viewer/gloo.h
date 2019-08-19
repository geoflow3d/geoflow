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

// System Headers
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Standard Headers
#include <string>
#include <iostream>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <initializer_list>
#include <limits>
#include <algorithm>
#include <array>

#include <imgui.h>

#include "../geoflow/common.hpp"

using namespace geoflow;

class Shader
{
public:

    // Implement Custom Constructor and Destructor
     Shader() { }
    ~Shader() { glDeleteProgram(mProgram); }

    void init();
    bool is_initialised(){ return initialised;};

    // Public Member Functions
    Shader & activate();
    Shader & attach(std::string const & filename);
    GLuint   create(std::string const & filename);
    GLuint   get() { return mProgram; }
    Shader & link();

    // Wrap Calls to glUniform
    // void bind(unsigned int location, float value);
    // void bind(unsigned int location, int value);
    // void bind(unsigned int location, glm::mat3 const & matrix);
    // void bind(unsigned int location, glm::mat4 const & matrix);
    // void bind(unsigned int location, glm::vec4 const & vector);
    // void bind(unsigned int location, glm::vec3 const & vector);

    void bind(unsigned int location, float value) 
    {   
        glUniform1f(location, value); 
    }
    void bind(unsigned int location, int value) 
    { 
        glUniform1i(location, value); 
    }
    void bind(unsigned int location, glm::mat4 const & matrix)
    { 
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix)); 
    }
    void bind(unsigned int location, glm::mat3 const & matrix)
    { 
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix)); 
    }
    void bind(unsigned int location, glm::vec4 const & vector)
    { 
        glUniform4fv(location, 1, glm::value_ptr(vector)); 
    }
    void bind(unsigned int location, glm::vec3 const & vector)
    { 
        glUniform3fv(location, 1, glm::value_ptr(vector)); 
    }
    template<typename T> Shader & bind(std::string const & name, T&& value)
    {
        int location = glGetUniformLocation(mProgram, name.c_str());
        if (location == -1) fprintf(stderr, "Missing Uniform: %s\n", name.c_str());
        else bind(location, std::forward<T>(value));
        return *this;
    }

private:

    // Disable Copying and Assignment
    Shader(Shader const &) = delete;
    Shader & operator=(Shader const &) = delete;

    void build();

    // Private Member Variables
    GLuint mProgram=0;
    GLint  mStatus;
    GLint  mLength;
    std::vector<std::string> sources;

    bool initialised=false;

};

class Buffer
{
public:
    Buffer() { }
    ~Buffer() { glDeleteBuffers(1, &mBuffer); }

    void init();
    bool is_initialised(){ return initialised;};
    void activate();
    void deactivate();
    GLuint get() { return mBuffer; }
    size_t element_bytesize() { return element_size; }

    // void add_field(size_t dim);
    // std::vector<size_t> get_fields();
    size_t get_stride();
    size_t get_length();

    template<typename T> void reserve_data(size_t length_, size_t dim);
    template<typename T> void set_data(T* data, size_t length, size_t stride);
    template<typename T> void set_subdata(T* data, size_t offset, size_t length);

private:
    GLfloat* data;
    GLuint mBuffer=0;
    size_t element_size, length=0, stride=0;
    // name, type, dim
    std::vector<size_t> data_fields;

    bool initialised=false;
    bool has_data=false;
};

class Texture1D
{
    public:
    Texture1D(){}
    ~Texture1D(){
        if(initialised)
            glDeleteTextures(1, &mTexture);
    }
    void activate();
    void deactivate();
    void init();
    bool is_initialised(){ return initialised;};
    void set_data(unsigned char * image, int width);
    void set_interpolation_nearest(){interpolation_nearest=true;};
    void set_interpolation_linear(){interpolation_nearest=false;};
    void set_wrap_repeat(){wrap_repeat=true;};
    void set_wrap_clamp(){wrap_repeat=false;};

    private:
    // GLint width;
    bool interpolation_nearest=false;
    bool wrap_repeat=false;
    GLuint mTexture;
    bool initialised=false;
};

class Uniform
{
    protected:
    const std::string name;
    public:
    Uniform(const std::string name):name(name){}
    virtual ~Uniform(){};
    virtual void gui(){};
    virtual void bind(Shader &s)=0;

    const std::string& get_name(){return name;};
};

// need to create trait-based solution for setting value of uniform (it should cast base uniform to specific one for datatype)
inline float slider_speed(float min, float max) {
    if (max!=min) return std::abs(max-min)/150;
    return 0.2;
}
class Uniform1f:public Uniform
{
    float def, min, max;
    float v_speed;
    float value;
    public:
    Uniform1f(const std::string name, float def=0, float min=0, float max=0)
    :Uniform(name), def(def), min(min), max(max), value(def) {
        v_speed = slider_speed(min,max);
    }
    void gui(){
        ImGui::PushID(this);
        ImGui::DragFloat(name.c_str(), &value, v_speed, min, max);
        ImGui::PopID();
    }
    void set_value(float v) {value = v;};
    float& get_value() {return value;};
    virtual void bind(Shader &s){
        s.bind(name, value);
    }
};
class Uniform1i:public Uniform
{
    int value = 1;
    public:
    using Uniform::Uniform;
    void gui(){
        ImGui::PushID(this);
        ImGui::SliderInt(name.c_str(), &value, 0, 3);
        ImGui::PopID();
    }
    virtual void bind(Shader &s){
        s.bind(name, value);
    }
};

class Uniform4f:public Uniform
{
    glm::vec4 value = glm::vec4(0.85,0.85,0.85,1.0);
    public:
    using Uniform::Uniform;
    void gui(){
        ImGui::PushID(this);
        ImGui::ColorEdit4(name.c_str(), glm::value_ptr(value), ImGuiColorEditFlags_NoInputs);
        ImGui::PopID();
    }
    virtual void bind(Shader &s){
        s.bind(name, value);
    }
};
class Uniform3f:public Uniform
{
    glm::vec3 value;
    float min, max, v_speed;
    public:
    Uniform3f(const std::string name, glm::vec3 def=glm::vec3(1), float min=0, float max=0)
    :Uniform(name), min(min), max(max), value(def) {
        v_speed = slider_speed(min,max);
    }
    void gui(){
        ImGui::PushID(this);
        ImGui::DragFloat3(name.c_str(), glm::value_ptr(value), v_speed, min, max);
        ImGui::PopID();
    }
    virtual void bind(Shader &s){
        s.bind(name, value);
    }
};

class BasePainter
{
    protected:
    BasePainter(BasePainter const &) = delete;
    BasePainter & operator=(BasePainter const &) = delete;
    // void set_buffer(std::unique_ptr<Buffer> b);
    // void set_program(std::unique_ptr<Shader> s);
    void setup_VertexArray();
    GLuint mVertexArray=0;
    int draw_mode;
    int polygon_mode = GL_FILL;
    // std::unique_ptr<Buffer> buffer = std::make_unique<Buffer>();
    std::unique_ptr<Shader> shader = std::make_unique<Shader>();
    // std::unordered_<std::string,float> uniforms;
    std::vector<std::unique_ptr<Uniform>> uniforms;
    std::unordered_map<std::string, std::unique_ptr<Buffer>> attributes;
    
    bool initialised=false;
    
    public:

    BasePainter(){};
    ~BasePainter() { glDeleteVertexArrays(1, &mVertexArray); }

    bool is_initialised() { return initialised; };
    // GLuint get() { return mVertexArray; }

    void attach_shader(std::string const & filename);
    // void set_data(GLfloat* data, size_t n, size_t stride);
    virtual void set_attribute(std::string name, GLfloat* data, size_t n, size_t stride);
    void enable_attribute(const std::string name);
    void disable_attribute(const std::string name);
    // void set_texture(unsigned char * image, int width);
    // void set_uniform(std::string const & name, GLfloat value);
    // float * get_uniform(std::string const & name);
    void set_drawmode(int dm) {draw_mode = dm;}
    int get_drawmode() {return draw_mode;}

    virtual void init();
    virtual void gui() {};
    virtual void render(glm::mat4 & model, glm::mat4 & view, glm::mat4 & projection)=0;
    
    // how to deal with uniforms?
    // float pointsize=1;

};

class hudPainter : public BasePainter {
    void init();
    public:
    hudPainter();
    void render(glm::mat4 & model, glm::mat4 & view, glm::mat4 & projection);
};

class Painter : public BasePainter {

    public:
    geoflow::Box& get_bbox(){
        return bbox;
    }
    void set_attribute(std::string name, GLfloat* data, size_t n, size_t stride);
    bool has_subdata();
    void set_geometry(GeometryCollection<vec3f>& geoms);
    void set_geometry(GeometryCollection<arr3f>& geoms);
    void set_geometry(GeometryCollection< std::array<arr3f,3> >& geoms);
    void set_geometry(GeometryCollection< std::array<arr3f,2> >& geoms);
    
    void begin_sub_attributes(std::string& name, size_t element_count, size_t stride);
    void set_sub_attributes(std::string& name, GLfloat* data, size_t count, size_t& offset);
    void end_sub_attributes(std::string& name);
    void begin_sub_geometries(size_t vertex_count, size_t dim);
    void set_sub_geometry(Geometry& geom, size_t& offset);
    void end_sub_geometries();
    
    void clear_attribute(const std::string name);

    void set_texture(std::weak_ptr<Texture1D> tex);
    void register_uniform(std::shared_ptr<Uniform> uniform);
    void unregister_uniform(std::shared_ptr<Uniform> uniform);
    void remove_texture();
    void clear_uniforms();
    void render(glm::mat4 & model, glm::mat4 & view, glm::mat4 & projection);
    void gui();
    void short_gui();
    

    private:
    std::vector<std::pair<size_t,size_t>> subdata_pairs;
    void init();
    geoflow::Box bbox;
    std::weak_ptr<Texture1D> texture;
    std::unordered_map<std::string, std::shared_ptr<Uniform>> uniforms_external;
};