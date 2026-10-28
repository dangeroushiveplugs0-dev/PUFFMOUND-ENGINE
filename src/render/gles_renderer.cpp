#include "spank3d/render/gles_renderer.hpp"

#include <GLES3/gl3.h>

#include <array>
#include <cstddef>

namespace spank3d::render {
namespace {

constexpr char kTriangleVertexShader[] = R"(#version 300 es
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec3 a_color;
out vec3 v_color;
void main() {
    v_color = a_color;
    gl_Position = vec4(a_position, 0.0, 1.0);
}
)";

constexpr char kTriangleFragmentShader[] = R"(#version 300 es
precision mediump float;
in vec3 v_color;
out vec4 out_color;
void main() {
    out_color = vec4(v_color, 1.0);
}
)";

struct TriangleVertex {
    float x;
    float y;
    float r;
    float g;
    float b;
};

constexpr std::array<TriangleVertex, 3> kTriangleVertices{{
    { 0.0F,  0.62F, 0.98F, 0.45F, 0.22F},
    {-0.58F, -0.48F, 0.20F, 0.78F, 0.98F},
    { 0.58F, -0.48F, 0.48F, 0.92F, 0.48F},
}};

GLuint compile_shader(GLenum type, const char* source) {
    const GLuint shader = glCreateShader(type);
    if (shader == 0) return 0;

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint create_triangle_program() {
    const GLuint vertex = compile_shader(GL_VERTEX_SHADER, kTriangleVertexShader);
    if (vertex == 0) return 0;

    const GLuint fragment = compile_shader(GL_FRAGMENT_SHADER, kTriangleFragmentShader);
    if (fragment == 0) {
        glDeleteShader(vertex);
        return 0;
    }

    const GLuint program = glCreateProgram();
    if (program == 0) {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return 0;
    }

    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    if (linked != GL_TRUE) {
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

} // namespace

bool GlesRenderer::initialize() {
    // Android must provide a current EGL context before this is called.
    if (glGetString(GL_VERSION) == nullptr) return false;

    shutdown();
    triangle_program_ = create_triangle_program();
    if (triangle_program_ == 0) {
        shutdown();
        return false;
    }

    glGenVertexArrays(1, &triangle_vao_);
    glGenBuffers(1, &triangle_vbo_);
    if (triangle_vao_ == 0 || triangle_vbo_ == 0) {
        shutdown();
        return false;
    }

    glBindVertexArray(triangle_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kTriangleVertices),
                 kTriangleVertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TriangleVertex),
                          reinterpret_cast<const void*>(offsetof(TriangleVertex, x)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleVertex),
                          reinterpret_cast<const void*>(offsetof(TriangleVertex, r)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    initialized_ = (glGetError() == GL_NO_ERROR);
    if (!initialized_) shutdown();
    return initialized_;
}

void GlesRenderer::resize(const Viewport& viewport) {
    viewport_ = viewport;
    if (initialized_) {
        glViewport(0, 0, static_cast<GLsizei>(viewport.width),
                   static_cast<GLsizei>(viewport.height));
    }
}

void GlesRenderer::begin_frame(const ClearColor& color) {
    if (!initialized_) return;
    glViewport(0, 0, static_cast<GLsizei>(viewport_.width),
               static_cast<GLsizei>(viewport_.height));
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GlesRenderer::draw_debug_triangle() {
    if (!initialized_) return;
    glUseProgram(triangle_program_);
    glBindVertexArray(triangle_vao_);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(kTriangleVertices.size()));
    glBindVertexArray(0);
    glUseProgram(0);
}

void GlesRenderer::end_frame() {
    // EGL buffer swapping is owned by the Android host/surface lifecycle.
}

void GlesRenderer::shutdown() noexcept {
    // Call while the EGL context that created these objects is still current.
    if (triangle_vbo_ != 0) glDeleteBuffers(1, &triangle_vbo_);
    if (triangle_vao_ != 0) glDeleteVertexArrays(1, &triangle_vao_);
    if (triangle_program_ != 0) glDeleteProgram(triangle_program_);
    triangle_vbo_ = 0;
    triangle_vao_ = 0;
    triangle_program_ = 0;
    initialized_ = false;
}

const char* GlesRenderer::backend_name() const noexcept {
    return "OpenGL ES 3";
}

} // namespace spank3d::render
