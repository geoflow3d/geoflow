// Local Headers
#include "gloo.h"

// Standard Headers
#include <cassert>
#include <fstream>
#include <memory>
#include <iostream>

// Define Namespace

void Shader::init() {
    mProgram = glCreateProgram();
    build();
    link();
    initialised = true;
}

Shader & Shader::activate()
{
    glUseProgram(mProgram);
    return *this;
}

void Shader::bind(unsigned int location, float value) 
{ 
    glUniform1f(location, value); 
}
void Shader::bind(unsigned int location, int value) 
{ 
    glUniform1i(location, value); 
}
void Shader::bind(unsigned int location, glm::mat4 const & matrix)
{ 
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix)); 
}
void Shader::bind(unsigned int location, glm::vec4 const & vector)
{ 
    glUniform4fv(location, 1, glm::value_ptr(vector)); 
}

Shader & Shader::attach(std::string const & filename)
{
    // Load GLSL Shader Source from File
    std::string path = PROJECT_SOURCE_DIR "/src/viewer/shaders/";
    path += filename;
    sources.push_back(path);
    return *this;
}

void Shader::build()
{
    for(auto filename:sources){
        std::ifstream fd(filename);
        auto src = std::string(std::istreambuf_iterator<char>(fd),
                            (std::istreambuf_iterator<char>()));

        // Create a Shader Object
        const char * source = src.c_str();
        auto shader = create(filename);
        glShaderSource(shader, 1, & source, nullptr);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, & mStatus);

        // Display the Build Log on Error
        if (mStatus == false)
        {
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, & mLength);
            std::unique_ptr<char[]> buffer(new char[mLength]);
            glGetShaderInfoLog(shader, mLength, nullptr, buffer.get());
            fprintf(stderr, "%s\n%s", filename.c_str(), buffer.get());
        }

        // Attach the Shader and Free Allocated Memory
        glAttachShader(mProgram, shader);
        glDeleteShader(shader);
    }
}

GLuint Shader::create(std::string const & filename)
{
    auto index = filename.rfind(".");
    auto ext = filename.substr(index + 1);
    if (ext == "frag") return glCreateShader(GL_FRAGMENT_SHADER);
    else if (ext == "geom") return glCreateShader(GL_GEOMETRY_SHADER);
    else if (ext == "vert") return glCreateShader(GL_VERTEX_SHADER);
    else                    return false;
}

Shader & Shader::link()
{
    glLinkProgram(mProgram);
    glGetProgramiv(mProgram, GL_LINK_STATUS, & mStatus);
    if(mStatus == false)
    {
        glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, & mLength);
        std::unique_ptr<char[]> buffer(new char[mLength]);
        glGetProgramInfoLog(mProgram, mLength, nullptr, buffer.get());
        fprintf(stderr, "%s", buffer.get());
    }
    assert(mStatus == true);
    return *this;
}



void Buffer::init()
{   
    if (mBuffer==0)
        glGenBuffers(1, &mBuffer);
    activate();
    glBufferData(GL_ARRAY_BUFFER, element_size*length, data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    initialised = true;
}

void Buffer::activate()
{
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
}

// void Buffer::add_field(size_t dim) {
//     data_fields.push_back(dim);
//     stride = stride+dim;
// }

std::vector<size_t> Buffer::get_fields() {
    return data_fields;
}

size_t Buffer::get_stride() {
    return stride;
}
size_t Buffer::get_length() {
    return length;
}

template<typename T> void Buffer::set_data(T* d, size_t n, std::initializer_list<int> dims)
{
    data = d;
    element_size = sizeof(T);
    length = n;
    
    data_fields.clear();
    stride = 0;
    for(auto dim:dims){
        data_fields.push_back(dim);
        stride += dim;
    }
    initialised = false;
}
template void Buffer::set_data(GLfloat*, size_t, std::initializer_list<int>);
// template void Buffer::set_data(double*, size_t);

Painter::Painter() {
    uniforms.push_back(std::unique_ptr<Uniform>(new Uniform1f("u_pointsize")));
    uniforms.push_back(std::unique_ptr<Uniform>(new Uniform1i("u_color_mode")));
    uniforms.push_back(std::unique_ptr<Uniform>(new Uniform4fv("u_color")));
    attributes["position"] = std::make_unique<Buffer>();
}

void Painter::init()
{
    if(mVertexArray==0)
        glGenVertexArrays(1, &mVertexArray);
    // setup_VertexArray();
    // if (buffer->is_initialised && shader->is_initialised)
    // if (draw_mode==GL_POINTS)
    //     set_uniform("u_pointsize",1.0);
    initialised = true;
}

void Painter::attach_shader(std::string const & filename)
{
    shader->attach(filename);
}
// void Painter::set_data(GLfloat* data, size_t n, std::initializer_list<int> dims)
// {
//     buffer->set_data(data, n, dims);
// }

// void Painter::set_buffer(std::unique_ptr<Buffer> b)
// {
//     buffer.swap(b);
// }
// void Painter::set_program(std::unique_ptr<Shader> s)
// {
//     shader.swap(s);
// }

void Painter::set_attribute(std::string name, GLfloat* data, size_t n, std::initializer_list<int> dims) {
    if(name == "position") {
        bbox.clear();
        for(size_t i=0; i<n/3; i++) {
            bbox.add(&data[i*3]);
        }
        std::cout << bbox.center()[0] << " " << bbox.center()[1] << " " << bbox.center()[2] << "\n";
    }
    attributes[name]->set_data(data, n, dims);
}

void Painter::setup_VertexArray()
{
    glBindVertexArray(mVertexArray);

    int i=0;
    for(auto& a : attributes) {
        a.second->activate();
        auto& buffer = a.second;
        auto& name = a.first;
        auto stride = buffer->get_stride();
        auto loc = glGetAttribLocation(shader->get(), name.c_str());
        glVertexAttribPointer(loc, stride, GL_FLOAT, GL_FALSE, stride * buffer->element_bytesize(), nullptr);
        glEnableVertexAttribArray(loc);
    }
    // // Position attribute
    // int start=0, i=0;
    // int stride = buffer->get_stride();
    // for(int dim:buffer->get_fields()) {
    //     glVertexAttribPointer(i, dim, GL_FLOAT, GL_FALSE, stride * buffer->element_bytesize(), (GLvoid*)(start * buffer->element_bytesize()));
    //     glEnableVertexAttribArray(i++);
    //     start = start+dim;
    // }

    glBindVertexArray(0); // Unbind VAO
}

// void Painter::set_uniform(std::string const & name, GLfloat value) {
//     uniforms[name] = value;
// }

// float * Painter::get_uniform(std::string const & name) {
//     return &uniforms[name];
// }

void Painter::gui() {
    ImGui::PushID(this);
    auto c = bbox.center();
    ImGui::Text("[%.2f, %.2f, %.2f]", c.x, c.y, c.z);

    const char* items[] = { "GL_POINTS", "GL_LINES", "GL_TRIANGLES", "GL_LINE_STRIP", "GL_LINE_LOOP" };
    const char* item_current;
    if(draw_mode==GL_POINTS) item_current=items[0];
    else if(draw_mode==GL_LINES) item_current=items[1];
    else if(draw_mode==GL_TRIANGLES) item_current=items[2];
    else if(draw_mode==GL_LINE_STRIP) item_current=items[3];
    else if(draw_mode==GL_LINE_LOOP) item_current=items[4];
    
    if (ImGui::BeginCombo("drawmode", item_current)) // The second parameter is the label previewed before opening the combo.
    {
      for (int n = 0; n < IM_ARRAYSIZE(items); n++)
      {
        bool is_selected = (item_current == items[n]);
        if (ImGui::Selectable(items[n], is_selected))
          item_current = items[n];
          ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
        if (item_current==items[0])
          set_drawmode(GL_POINTS);
        else if (item_current==items[1])
          set_drawmode(GL_LINES);
        else if (item_current==items[2])
          set_drawmode(GL_TRIANGLES);
        else if (item_current==items[3])
          set_drawmode(GL_LINE_STRIP);
        else if (item_current==items[4])
          set_drawmode(GL_LINE_LOOP);
      }
        ImGui::EndCombo();
    }
    for (auto &u: uniforms) {
        if(u->get_name()=="u_pointsize" && get_drawmode()!=GL_POINTS)
            continue;
        else
            u->gui();
    }
    ImGui::PopID();
}

void Painter::render(glm::mat4 & model, glm::mat4 & view, glm::mat4 & projection)
{
    if(!shader->is_initialised())
        shader->init();
    if(!initialised)
        init();
    for(auto& a : attributes) {
        if(!a.second->is_initialised()) {
            a.second->init();
            setup_VertexArray();
        }
    }
    
    shader->activate();

    // note all shaders have these! eg crosshair painter
    shader->bind("u_projection", projection);
    shader->bind("u_view", view);
    shader->bind("u_model", model);

    for (auto &u: uniforms) {
        u->bind(*shader);
    }

    glBindVertexArray(mVertexArray);
    for(auto& a : attributes) {
        glDrawArrays(draw_mode, 0, a.second->get_length()/a.second->get_stride());
    }
    glBindVertexArray(0);
}
