#pragma once

// System Headers
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Standard Headers
#include <string>
#include <memory>
#include <tuple>
#include <map>
#include <vector>
#include <initializer_list>

// Define Namespace
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
    size_t element_size, length, stride=0;
    // name, type, dim
    std::vector<size_t> data_fields;

    bool initialised=false;
};

class Painter
{
public:
    Painter() {}
    ~Painter() { glDeleteVertexArrays(1, &mVertexArray); }

    void init();
    bool is_initialised(){ return initialised;};
    // GLuint get() { return mVertexArray; }

    void attach_shader(std::string const & filename);
    void set_data(GLfloat* data, size_t n, std::initializer_list<int> dims);
    void set_uniform(std::string const & name, GLfloat value);
    float * get_uniform(std::string const & name);
    void set_drawmode(int dm) {draw_mode = dm;}
    int get_drawmode() {return draw_mode;}

    void render(glm::mat4 & model, glm::mat4 & view, glm::mat4 & projection);
    
    // how to deal with uniforms?
    // float pointsize=1;
private:
    // void set_buffer(std::unique_ptr<Buffer> b);
    // void set_program(std::unique_ptr<Shader> s);
    void setup_VertexArray();
    GLuint mVertexArray=0;
    int draw_mode;
    std::unique_ptr<Buffer> buffer = std::make_unique<Buffer>();
    std::unique_ptr<Shader> shader = std::make_unique<Shader>();
    std::map<std::string,float> uniforms;

    bool initialised=false;
};