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
    std::ifstream fd(path + filename);
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
    return *this;
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
    glGenBuffers(1, &mBuffer);
}

void Buffer::activate()
{
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
}

template<typename T> void Buffer::set_data(T* data, size_t n)
{
    activate();
    element_size = sizeof(T);
    glBufferData(GL_ARRAY_BUFFER, element_size*n, data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
template void Buffer::set_data(float*, size_t);
template void Buffer::set_data(double*, size_t);


void Painter::init()
{
    glGenVertexArrays(1, &mVertexArray);
}

void Painter::set_buffer(Buffer& b)
{
    buffer = &b;
}
void Painter::set_program(Shader& s)
{
    shader = &s;
}

void Painter::setup_VertexArray()
{
    glBindVertexArray(mVertexArray);
    buffer->activate();

    // // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * buffer->size(), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * buffer->size(), (GLvoid*)(3 * buffer->size()));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); // Unbind VAO
}

void Painter::render(glm::mat4 & model, glm::mat4 & view, glm::mat4 & projection)
{
    shader->activate();

    GLint projLoc = glGetUniformLocation(shader->get(), "u_projection"); 
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    GLint viewLoc = glGetUniformLocation(shader->get(), "u_view"); 
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    GLint modLoc = glGetUniformLocation(shader->get(), "u_model"); 
    glUniformMatrix4fv(modLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(mVertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    // std::cout << "draw" <<std::endl;
}