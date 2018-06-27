#pragma once

// System Headers
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Standard Headers
#include <string>
#include <string>

// Define Namespace
class Shader
{
public:

    // Implement Custom Constructor and Destructor
     Shader() { }
    ~Shader() { glDeleteProgram(mProgram); }

    void init();

    // Public Member Functions
    Shader & activate();
    Shader & attach(std::string const & filename);
    GLuint   create(std::string const & filename);
    GLuint   get() { return mProgram; }
    Shader & link();

    // Wrap Calls to glUniform
    void bind(unsigned int location, float value);
    void bind(unsigned int location, glm::mat4 const & matrix);
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

    // Private Member Variables
    GLuint mProgram;
    GLint  mStatus;
    GLint  mLength;

};

class Buffer
{
public:
    Buffer() { }
    ~Buffer() { glDeleteBuffers(1, &mBuffer); }

    void init();
    void activate();
    GLuint get() { return mBuffer; }

    template<typename T> void set_data(T* data, size_t n);

private:
    GLuint mBuffer;
};

class Painter
{
public:
    Painter() { }
    ~Painter() { glDeleteVertexArrays(1, &mVertexArray); }

    void init();
    GLuint get() { return mVertexArray; }

    void set_buffer(Buffer& b);
    void set_program(Shader& s);

    void setup_VertexArray();

    void render(glm::mat4 & model, glm::mat4 & view, glm::mat4 & projection);

private:
    GLuint mVertexArray;
    Buffer * buffer;
    Shader * shader;
};