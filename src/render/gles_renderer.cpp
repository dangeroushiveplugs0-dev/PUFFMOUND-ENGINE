#include "spank3d/render/gles_renderer.hpp"

#include <GLES3/gl3.h>

#include <array>
#include <cmath>
#include <algorithm>
#include <cstddef>
#include <cstdio>

namespace spank3d::render {
namespace {

constexpr char kCubeVertexShader[] = R"(#version 300 es
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_color;
uniform mat4 u_mvp;
out vec3 v_color;
void main() {
    v_color = a_color;
    gl_Position = u_mvp * vec4(a_position, 1.0);
}
)";

constexpr char kCubeFragmentShader[] = R"(#version 300 es
precision mediump float;
in vec3 v_color;
out vec4 out_color;
void main() {
    out_color = vec4(v_color, 1.0);
}
)";

struct CubeVertex {
    float x, y, z;
    float r, g, b;
};

#define V(x, y, z, r, g, b) CubeVertex{x, y, z, r, g, b}
constexpr std::array<CubeVertex, 36> kCubeVertices{{
    // Front (+Z)
    V(-1,-1, 1, 1.00F,0.32F,0.26F), V( 1,-1, 1, 1.00F,0.32F,0.26F), V( 1, 1, 1, 1.00F,0.32F,0.26F),
    V(-1,-1, 1, 1.00F,0.32F,0.26F), V( 1, 1, 1, 1.00F,0.32F,0.26F), V(-1, 1, 1, 1.00F,0.32F,0.26F),
    // Back (-Z)
    V( 1,-1,-1, 0.26F,0.66F,1.00F), V(-1,-1,-1, 0.26F,0.66F,1.00F), V(-1, 1,-1, 0.26F,0.66F,1.00F),
    V( 1,-1,-1, 0.26F,0.66F,1.00F), V(-1, 1,-1, 0.26F,0.66F,1.00F), V( 1, 1,-1, 0.26F,0.66F,1.00F),
    // Right (+X)
    V(1,-1, 1, 0.35F,0.88F,0.57F), V(1,-1,-1, 0.35F,0.88F,0.57F), V(1, 1,-1, 0.35F,0.88F,0.57F),
    V(1,-1, 1, 0.35F,0.88F,0.57F), V(1, 1,-1, 0.35F,0.88F,0.57F), V(1, 1, 1, 0.35F,0.88F,0.57F),
    // Left (-X)
    V(-1,-1,-1, 0.98F,0.72F,0.24F), V(-1,-1, 1, 0.98F,0.72F,0.24F), V(-1, 1, 1, 0.98F,0.72F,0.24F),
    V(-1,-1,-1, 0.98F,0.72F,0.24F), V(-1, 1, 1, 0.98F,0.72F,0.24F), V(-1, 1,-1, 0.98F,0.72F,0.24F),
    // Top (+Y)
    V(-1,1, 1, 0.70F,0.42F,0.96F), V( 1,1, 1, 0.70F,0.42F,0.96F), V( 1,1,-1, 0.70F,0.42F,0.96F),
    V(-1,1, 1, 0.70F,0.42F,0.96F), V( 1,1,-1, 0.70F,0.42F,0.96F), V(-1,1,-1, 0.70F,0.42F,0.96F),
    // Bottom (-Y)
    V(-1,-1,-1, 0.22F,0.82F,0.86F), V( 1,-1,-1, 0.22F,0.82F,0.86F), V( 1,-1, 1, 0.22F,0.82F,0.86F),
    V(-1,-1,-1, 0.22F,0.82F,0.86F), V( 1,-1, 1, 0.22F,0.82F,0.86F), V(-1,-1, 1, 0.22F,0.82F,0.86F)
}};
#undef V

using Mat4 = std::array<float, 16>;

Mat4 identity() {
    return {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
}

Mat4 multiply(const Mat4& a, const Mat4& b) {
    Mat4 out{};
    for (int col = 0; col < 4; ++col)
        for (int row = 0; row < 4; ++row)
            for (int k = 0; k < 4; ++k)
                out[col * 4 + row] += a[k * 4 + row] * b[col * 4 + k];
    return out;
}

Mat4 rotation_x(float a) {
    Mat4 m = identity();
    const float c = std::cos(a), s = std::sin(a);
    m[5] = c; m[9] = -s; m[6] = s; m[10] = c;
    return m;
}

Mat4 rotation_y(float a) {
    Mat4 m = identity();
    const float c = std::cos(a), s = std::sin(a);
    m[0] = c; m[8] = s; m[2] = -s; m[10] = c;
    return m;
}

Mat4 translation(float x, float y, float z) {
    Mat4 m = identity();
    m[12] = x; m[13] = y; m[14] = z;
    return m;
}

Mat4 perspective(float fov, float aspect, float near_plane, float far_plane) {
    Mat4 m{};
    const float f = 1.0F / std::tan(fov * 0.5F);
    m[0] = f / aspect;
    m[5] = f;
    m[10] = (far_plane + near_plane) / (near_plane - far_plane);
    m[11] = -1.0F;
    m[14] = (2.0F * far_plane * near_plane) / (near_plane - far_plane);
    return m;
}

Mat4 orthographic(float half_height, float aspect, float near_plane, float far_plane) {
    const float half_width = half_height * aspect;
    Mat4 m = identity();
    m[0] = 1.0F / half_width;
    m[5] = 1.0F / half_height;
    m[10] = -2.0F / (far_plane - near_plane);
    m[14] = -(far_plane + near_plane) / (far_plane - near_plane);
    return m;
}

GLuint compile_shader(GLenum type, const char* source) {
    const GLuint shader = glCreateShader(type);
    if (shader == 0) return 0;
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        std::array<GLchar, 1024> log{};
        GLsizei length = 0;
        glGetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), &length, log.data());
        std::fprintf(stderr, "Spank3d: GLSL shader compilation failed: %.*s\n",
                     static_cast<int>(length), log.data());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint create_cube_program() {
    const GLuint vertex = compile_shader(GL_VERTEX_SHADER, kCubeVertexShader);
    if (!vertex) return 0;
    const GLuint fragment = compile_shader(GL_FRAGMENT_SHADER, kCubeFragmentShader);
    if (!fragment) {
        glDeleteShader(vertex);
        return 0;
    }
    const GLuint program = glCreateProgram();
    if (!program) {
        glDeleteShader(vertex); glDeleteShader(fragment);
        return 0;
    }
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    glDeleteShader(vertex); glDeleteShader(fragment);
    if (linked != GL_TRUE) {
        std::array<GLchar, 1024> log{};
        GLsizei length = 0;
        glGetProgramInfoLog(program, static_cast<GLsizei>(log.size()), &length, log.data());
        std::fprintf(stderr, "Spank3d: GLSL program link failed: %.*s\n",
                     static_cast<int>(length), log.data());
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

} // namespace

bool GlesRenderer::initialize() {
    if (glGetString(GL_VERSION) == nullptr) return false;
    while (glGetError() != GL_NO_ERROR) {}
    shutdown();

    cube_program_ = create_cube_program();
    if (!cube_program_) return false;
    mvp_location_ = glGetUniformLocation(cube_program_, "u_mvp");
    if (mvp_location_ < 0) {
        shutdown();
        return false;
    }

    glGenVertexArrays(1, &cube_vao_);
    glGenBuffers(1, &cube_vbo_);
    if (!cube_vao_ || !cube_vbo_) {
        shutdown();
        return false;
    }

    glBindVertexArray(cube_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kCubeVertices), kCubeVertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CubeVertex),
                          reinterpret_cast<const void*>(offsetof(CubeVertex, x)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CubeVertex),
                          reinterpret_cast<const void*>(offsetof(CubeVertex, r)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    initialized_ = glGetError() == GL_NO_ERROR;
    if (!initialized_) shutdown();
    return initialized_;
}

void GlesRenderer::resize(const Viewport& viewport) {
    viewport_ = viewport;
    if (initialized_)
        glViewport(0, 0, static_cast<GLsizei>(viewport.width), static_cast<GLsizei>(viewport.height));
}

void GlesRenderer::begin_frame(const ClearColor& color) {
    if (!initialized_) return;
    glViewport(0, 0, static_cast<GLsizei>(viewport_.width), static_cast<GLsizei>(viewport_.height));
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GlesRenderer::draw_debug_cube(float angle_radians) {
    if (!initialized_ || viewport_.height == 0 || viewport_.width == 0) return;
    const Mat4 model = multiply(rotation_y(angle_radians), rotation_x(angle_radians * 0.63F));
    const Mat4 view = multiply(
        translation(0.0F, 0.0F, -camera_distance_),
        multiply(rotation_x(-camera_pitch_),
                 multiply(rotation_y(-camera_yaw_),
                          translation(-camera_target_x_, -camera_target_y_, 0.0F))));
    const float aspect = static_cast<float>(viewport_.width) / static_cast<float>(viewport_.height);
    const Mat4 projection = orthographic_
        ? orthographic(2.8F, aspect, 0.1F, 100.0F)
        : perspective(0.82F, aspect, 0.1F, 100.0F);
    const Mat4 mvp = multiply(projection, multiply(view, model));

    glUseProgram(cube_program_);
    glUniformMatrix4fv(mvp_location_, 1, GL_FALSE, mvp.data());
    glBindVertexArray(cube_vao_);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(kCubeVertices.size()));
    glBindVertexArray(0);
    glUseProgram(0);
}

void GlesRenderer::camera_orbit(float delta_x, float delta_y) noexcept {
    camera_yaw_ += delta_x * 0.018F;
    constexpr float limit = 1.48F;
    camera_pitch_ = std::clamp(camera_pitch_ + delta_y * 0.018F, -limit, limit);
}

void GlesRenderer::camera_pan(float delta_x, float delta_y) noexcept {
    const float scale = camera_distance_ * 3.2F;
    const float c = std::cos(camera_yaw_);
    const float s = std::sin(camera_yaw_);
    camera_target_x_ -= (delta_x * c) * scale;
    camera_target_x_ += (delta_y * s) * scale;
    camera_target_y_ += delta_y * scale;
}

void GlesRenderer::camera_zoom(float scale_factor) noexcept {
    if (!std::isfinite(scale_factor) || scale_factor <= 0.0F) return;
    camera_distance_ = std::clamp(camera_distance_ / std::pow(scale_factor, 1.25F), 2.0F, 24.0F);
}

void GlesRenderer::camera_toggle_projection() noexcept {
    orthographic_ = !orthographic_;
}

void GlesRenderer::camera_reset() noexcept {
    camera_yaw_ = 0.55F;
    camera_pitch_ = 0.32F;
    camera_distance_ = 4.8F;
    camera_target_x_ = 0.0F;
    camera_target_y_ = 0.0F;
    orthographic_ = false;
}

void GlesRenderer::end_frame() {}

void GlesRenderer::shutdown() noexcept {
    if (cube_vbo_) glDeleteBuffers(1, &cube_vbo_);
    if (cube_vao_) glDeleteVertexArrays(1, &cube_vao_);
    if (cube_program_) glDeleteProgram(cube_program_);
    cube_vbo_ = cube_vao_ = cube_program_ = 0;
    mvp_location_ = -1;
    initialized_ = false;
}

const char* GlesRenderer::backend_name() const noexcept { return "OpenGL ES 3"; }

} // namespace spank3d::render
