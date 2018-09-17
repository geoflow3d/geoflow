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

void Texture1D::activate() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, mTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    if(interpolation_nearest) {
        glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);    
    } else {
        glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}
void Texture1D::deactivate() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, 0);
}
void Texture1D::init(){
    glGenTextures(1, &mTexture);
    activate();
    initialised = true;
}
void Texture1D::set_data(unsigned char * image, int width){
    activate();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, width, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    // glBindTexture(GL_TEXTURE_1D, 0);
}

void Buffer::init()
{   
    if (mBuffer==0)
        glGenBuffers(1, &mBuffer);
    activate();
    initialised = true;
}

void Buffer::activate()
{
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
}
void Buffer::deactivate()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
    element_size = sizeof(T);
    length = n;
    
    data_fields.clear();
    stride = 0;
    for(auto dim:dims){
        data_fields.push_back(dim);
        stride += dim;
    }
    activate();
    glBufferData(GL_ARRAY_BUFFER, element_size*length, d, GL_DYNAMIC_DRAW);
    deactivate();
    has_data = true;
}
template void Buffer::set_data(GLfloat*, size_t, std::initializer_list<int>);
// template void Buffer::set_data(double*, size_t);


void BasePainter::init()
{
    if(mVertexArray==0)
        glGenVertexArrays(1, &mVertexArray);

    setup_VertexArray();

    initialised = true;
}

void BasePainter::attach_shader(std::string const & filename)
{
    shader->attach(filename);
}

void BasePainter::set_attribute(std::string name, GLfloat* data, size_t n, std::initializer_list<int> dims) {
    attributes[name]->set_data(data, n, dims);
    enable_attribute(name);
}
// void BasePainter::set_texture(unsigned char * image, int width) {
//     textures[0]->set_data(image, width);
// }

void BasePainter::enable_attribute(const std::string name) {
    setup_VertexArray();
    glBindVertexArray(mVertexArray);
    auto loc = glGetAttribLocation(shader->get(), name.c_str());
    glEnableVertexAttribArray(loc);
}
void BasePainter::disable_attribute(const std::string name) {
    glBindVertexArray(mVertexArray);
    auto loc = glGetAttribLocation(shader->get(), name.c_str());
    glDisableVertexAttribArray(loc);
}

void BasePainter::setup_VertexArray()
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
        a.second->deactivate();
        // glEnableVertexAttribArray(loc);
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

hudPainter::hudPainter() {

}

void hudPainter::init() {
    std::array<GLfloat,8> crosshair_lines = {
        -1, 0,
        1,  0,
        0, -1,
        0,  1
    };
    attributes["position"] = std::make_unique<Buffer>();
    set_attribute("position", crosshair_lines.data(), crosshair_lines.size(), {2});
    set_drawmode(GL_LINES);
    BasePainter::init();
}

void hudPainter::render(glm::mat4 & model, glm::mat4 & view, glm::mat4 & projection){
    if(!shader->is_initialised()){
        attach_shader("crosshair.vert");
        attach_shader("crosshair.frag");
        shader->init();
    }
    if(!initialised){
        init();
    }
    shader->activate();

    glBindVertexArray(mVertexArray);
    if (attributes["position"]->get_length()>0) {
        auto n = attributes["position"]->get_length()/attributes["position"]->get_stride();
        glDrawArrays(draw_mode, 0, n);
    }   
    glBindVertexArray(0);
}


void Painter::set_attribute(std::string name, GLfloat* data, size_t n, std::initializer_list<int> dims) {
    if(name == "position") {
        bbox.clear();
        for(size_t i=0; i<n/3; i++) {
            bbox.add(&data[i*3]);
        }
        std::cout << bbox.center()[0] << " " << bbox.center()[1] << " " << bbox.center()[2] << "\n";
    }

    attributes[name]->set_data(data, n, dims);
    enable_attribute(name);
}

void Painter::clear_attribute(const std::string name) {
    if(name == "position")
        bbox.clear();
    disable_attribute(name);
}

void Painter::set_texture(std::weak_ptr<Texture1D> tex) {
    texture = tex;
}
void Painter::register_uniform(std::shared_ptr<Uniform> uniform) {
    uniforms_external[uniform] = uniform;
}
void Painter::unregister_uniform(std::shared_ptr<Uniform> uniform) {
    uniforms_external.erase(uniform);
}
void Painter::remove_texture() {
    if (auto t = texture.lock()) {
        t->deactivate();
    }
    texture.reset();
}
void Painter::clear_uniforms() {
    uniforms_external.clear();
}

void Painter::gui() {
    ImGui::PushID(this);
    auto c = bbox.center();
    // ImGui::Text("Init: %d", is_initialised());
    ImGui::Text("[%.2f, %.2f, %.2f]", c.x, c.y, c.z);
    if(is_initialised()) {
        size_t n = 0;
        if (attributes["position"]->get_length()>0)
            n = attributes["position"]->get_length()/attributes["position"]->get_stride();
        ImGui::Text("[%zu vertices]", n);
    }
    {
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
    }
    if(draw_mode==GL_TRIANGLES) {
        const char* items[] = { "GL_POINT", "GL_LINE", "GL_FILL" };
        const char* item_current;
        if(polygon_mode==GL_POINT) item_current=items[0];
        else if(polygon_mode==GL_LINE) item_current=items[1];
        else if(polygon_mode==GL_FILL) item_current=items[2];
        
        if (ImGui::BeginCombo("polygonmode", item_current)) // The second parameter is the label previewed before opening the combo.
        {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            bool is_selected = (item_current == items[n]);
            if (ImGui::Selectable(items[n], is_selected))
            item_current = items[n];
            ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
            if (item_current==items[0])
                polygon_mode = GL_POINT;
            else if (item_current==items[1])
                polygon_mode = GL_LINE;
            else if (item_current==items[2])
                polygon_mode = GL_FILL;
        }
            ImGui::EndCombo();
        }
    }
    for (auto &u: uniforms) {
        if(u->get_name()=="u_pointsize" && !(get_drawmode()==GL_POINTS || (get_drawmode()==GL_TRIANGLES && polygon_mode==GL_POINT)))
            continue;
        else
            u->gui();
    }
    ImGui::PopID();
}

void Painter::init() {
    uniforms.push_back(std::unique_ptr<Uniform>(new Uniform1f("u_pointsize", 2,0,20)));
    uniforms.push_back(std::unique_ptr<Uniform>(new Uniform1i("u_color_mode")));
    uniforms.push_back(std::unique_ptr<Uniform>(new Uniform4f("u_color")));
    uniforms.push_back(std::unique_ptr<Uniform>(new Uniform1f("u_ambient", 0.65,0,1)));
    uniforms.push_back(std::unique_ptr<Uniform>(new Uniform1f("u_diffuse", 1.0,0,1)));
    uniforms.push_back(std::unique_ptr<Uniform>(new Uniform1f("u_specular", 0.5,0,1)));
    attributes["position"] = std::make_unique<Buffer>();
    attributes["normal"] = std::make_unique<Buffer>();
    attributes["value"] = std::make_unique<Buffer>();
    attributes["identifier"] = std::make_unique<Buffer>();

    for(auto& a : attributes) {
        if(!a.second->is_initialised()) {
            a.second->init();
        }
    }

    BasePainter::init();
}

void Painter::render(glm::mat4 & model, glm::mat4 & view, glm::mat4 & projection)
{
    if(!shader->is_initialised())
        shader->init();
    if(!initialised)
        init();

    if(auto t = texture.lock()) {
        if(!t->is_initialised()) t->init();
        t->activate();
    }
    shader->activate();

    // note all shaders have these! eg crosshair painter
    auto mvp = projection*view*model;
    shader->bind("u_mvp", mvp);
    shader->bind("u_mv_normal", glm::mat3(glm::transpose(glm::inverse(view*model))));
    // shader->bind("u_projection", projection);
    // shader->bind("u_view", view);
    // shader->bind("u_model", model);

    for (auto &u: uniforms) {
        u->bind(*shader);
    }
    for (auto u_ptr: uniforms_external) {
        u_ptr.second->bind(*shader);
    }
    if (draw_mode==GL_TRIANGLES)
        glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);

    glBindVertexArray(mVertexArray);
    if (attributes["position"]->get_length()>0) {
        auto n = attributes["position"]->get_length()/attributes["position"]->get_stride();
        glDrawArrays(draw_mode, 0, n);
    }   
    glBindVertexArray(0);
}
