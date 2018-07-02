#pragma once

// System Headers
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Standard Headers
#include <string>
#include <memory>

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
    void activate();
    GLuint get() { return mBuffer; }
    size_t element_bytesize() { return element_size; }

    void add_field(std::string name, DTYPE dtype, size_t dim);

    template<typename T> void set_data(T* data, size_t n);

private:
    GLuint mBuffer;
    size_t element_size;
    // name, type, dim
    std::vector<std::tuple<std::string, DTYPE, size_t>> data_structure;
};

class Painter
{
public:
    Painter() { }
    ~Painter() { glDeleteVertexArrays(1, &mVertexArray); }

    void init();
    GLuint get() { return mVertexArray; }

    void set_buffer(std::unique_ptr<Buffer> b);
    void set_program(std::unique_ptr<Shader> s);

    void setup_VertexArray();

    void render(glm::mat4 & model, glm::mat4 & view, glm::mat4 & projection);

private:
    GLuint mVertexArray;
    std::unique_ptr<Buffer> buffer;
    std::unique_ptr<Shader> shader;
};