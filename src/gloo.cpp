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
void Shader::bind(unsigned int location, glm::mat4 const & matrix)
{ 
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix)); 
}

Shader & Shader::attach(std::string const & filename)
{
    // Load GLSL Shader Source from File
    std::string path = PROJECT_SOURCE_DIR "/shaders/";
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
    if (!mBuffer)
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

void Buffer::add_field(size_t dim) {
    data_fields.push_back(dim);
    stride = stride+dim;
}

std::vector<size_t> Buffer::get_fields() {
    return data_fields;
}

size_t Buffer::get_stride() {
    return stride;
}
size_t Buffer::get_length() {
    return length;
}

template<typename T> void Buffer::set_data(T* d, size_t n)
{
    data = d;
    element_size = sizeof(T);
    length = n;
    initialised = false;
}
template void Buffer::set_data(float*, size_t);
// template void Buffer::set_data(double*, size_t);


void Painter::init()
{
    if(!mVertexArray)
        glGenVertexArrays(1, &mVertexArray);
    // setup_VertexArray();
    // if (buffer->is_initialised && shader->is_initialised)
    initialised = true;
}

void Painter::set_buffer(std::unique_ptr<Buffer> b)
{
    buffer.swap(b);
}
void Painter::set_program(std::unique_ptr<Shader> s)
{
    shader.swap(s);
}

void Painter::setup_VertexArray()
{
    glBindVertexArray(mVertexArray);
    buffer->activate();

    // // Position attribute
    int start=0, i=0;
    int stride = buffer->get_stride();
    for(int dim:buffer->get_fields()) {
        glVertexAttribPointer(i, dim, GL_FLOAT, GL_FALSE, stride * buffer->element_bytesize(), (GLvoid*)(start * buffer->element_bytesize()));
        glEnableVertexAttribArray(i++);
        start = start+dim;
    }

    glBindVertexArray(0); // Unbind VAO
}

void Painter::render(glm::mat4 & model, glm::mat4 & view, glm::mat4 & projection)
{
    if(!shader->is_initialised())
        shader->init();
    if(!initialised)
        init();
    if(!buffer->is_initialised()){
        buffer->init();
        setup_VertexArray();
    }
    shader->activate();

    GLint projLoc = glGetUniformLocation(shader->get(), "u_projection"); 
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    GLint viewLoc = glGetUniformLocation(shader->get(), "u_view"); 
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    GLint modLoc = glGetUniformLocation(shader->get(), "u_model"); 
    glUniformMatrix4fv(modLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(mVertexArray);
    glDrawArrays(draw_mode, 0, buffer->get_length()/buffer->get_stride());
    glBindVertexArray(0);
}