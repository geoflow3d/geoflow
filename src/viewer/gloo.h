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

class Box {
    private:
    std::array<float,3> pmin, pmax;
    bool just_cleared;
    public:
    Box(){
        std::cout << "CCC\n";
        clear();
    }
    void add(float p[]){
        if(just_cleared){
            pmin[0] = p[0];
            pmin[1] = p[1];
            pmin[2] = p[2];
            pmax[0] = p[0];
            pmax[1] = p[1];
            pmax[2] = p[2];
            just_cleared = false;
        }
        pmin[0] = std::min(p[0], pmin[0]);
        pmin[1] = std::min(p[1], pmin[1]);
        pmin[2] = std::min(p[2], pmin[2]);
        pmax[0] = std::max(p[0], pmax[0]);
        pmax[1] = std::max(p[1], pmax[1]);
        pmax[2] = std::max(p[2], pmax[2]);
        // std::cout << "adding point p = " << p[0] << ", " << p[1] << ", " << p[2] << "\n";
        // std::cout << "bb min = " << pmin[0] << ", " << pmin[1] << ", " << pmin[2] << "\n";
        // std::cout << "bb max = " << pmax[0] << ", " << pmax[1] << ", " << pmax[2] << "\n";
    }
    void add(Box& otherBox){
        add(otherBox.min().data());
        add(otherBox.max().data());
    }
    void clear(){
        pmin.fill(0);
        pmax.fill(0);
        just_cleared = true;
    }
    bool isEmpty(){
        return just_cleared;
    }
    glm::vec3 center(){
        return {(pmax[0]+pmin[0])/2, (pmax[1]+pmin[1])/2, (pmax[2]+pmin[2])/2};
    }
    std::array<float,3> min(){
        return pmin;
    }
    std::array<float,3> max(){
        return pmax;
    }
};

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
    void bind(unsigned int location, float value);
    void bind(unsigned int location, int value);
    void bind(unsigned int location, glm::mat4 const & matrix);
    void bind(unsigned int location, glm::vec4 const & vector);
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

enum DTYPE {
    FLOAT,
    DOUBLE
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
    std::vector<size_t> get_fields();
    size_t get_stride();
    size_t get_length();

    template<typename T> void set_data(T* data, size_t n, std::initializer_list<int> dims);

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
        glDeleteTextures(1, &mTexture);
    }
    void activate();
    void deactivate();
    void init();
    bool is_initialised(){ return initialised;};
    void set_data(unsigned char * image, int width);

    private:
    // GLint width;
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

class Uniform1f:public Uniform
{
    float value = 2.0;
    public:
    using Uniform::Uniform;
    void gui(){
        ImGui::PushID(this);
        ImGui::SliderFloat(name.c_str(), &value, 1, 30);
        ImGui::PopID();
    }
    void set_value(float v) {value = v;};
    float get_value() {return value;};
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
        ImGui::SliderInt(name.c_str(), &value, 0, 2);
        ImGui::PopID();
    }
    virtual void bind(Shader &s){
        s.bind(name, value);
    }
};

class Uniform4fv:public Uniform
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

class Painter
{
public:
    Painter();
    ~Painter() { glDeleteVertexArrays(1, &mVertexArray); }

    void init();
    bool is_initialised(){ return initialised;};
    // GLuint get() { return mVertexArray; }

    void attach_shader(std::string const & filename);
    // void set_data(GLfloat* data, size_t n, std::initializer_list<int> dims);
    void set_attribute(std::string name, GLfloat* data, size_t n, std::initializer_list<int> dims);
    void enable_attribute(const std::string name);
    void disable_attribute(const std::string name);
    void clear_attribute(const std::string name);

    void set_texture(std::weak_ptr<Texture1D> tex);
    void add_uniform(std::weak_ptr<Uniform> uniform);
    void remove_texture();
    void clear_uniforms();
    // void set_texture(unsigned char * image, int width);
    // void set_uniform(std::string const & name, GLfloat value);
    // float * get_uniform(std::string const & name);
    void set_drawmode(int dm) {draw_mode = dm;}
    int get_drawmode() {return draw_mode;}

    Box& get_bbox(){
        return bbox;
    }

    void gui();
    void render(glm::mat4 & model, glm::mat4 & view, glm::mat4 & projection);
    
    // how to deal with uniforms?
    // float pointsize=1;
private:
    Painter(Painter const &) = delete;
    Painter & operator=(Painter const &) = delete;
    // void set_buffer(std::unique_ptr<Buffer> b);
    // void set_program(std::unique_ptr<Shader> s);
    void setup_VertexArray();
    GLuint mVertexArray=0;
    int draw_mode;
    // std::unique_ptr<Buffer> buffer = std::make_unique<Buffer>();
    std::unique_ptr<Shader> shader = std::make_unique<Shader>();
    // std::unordered_<std::string,float> uniforms;
    std::vector<std::unique_ptr<Uniform>> uniforms;
    std::vector<std::weak_ptr<Uniform>> uniforms_external;
    std::unordered_map<std::string, std::unique_ptr<Buffer>> attributes;
    std::weak_ptr<Texture1D> texture;

    Box bbox;

    bool initialised=false;
};