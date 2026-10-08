#include <print>

#include <SDL3/SDL.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "generation/perlin_noise.hpp"
#include "opengl/shader_program.hpp"
#include "opengl/vertex_array.hpp"

struct VertexData {
    float x, y, z;
};

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window* window = SDL_CreateWindow("3dmq - OpenGL 4.5", 1280, 720,
                                          SDL_WINDOW_OPENGL);
    if (window == nullptr) {
        SDL_Quit();
        return 1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!gladLoadGL()) {
        SDL_GL_DestroyContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!SDL_GL_MakeCurrent(window, gl_context)) {
        SDL_GL_DestroyContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    const char* version =
        reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* renderer =
        reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* vendor =
        reinterpret_cast<const char*>(glGetString(GL_VENDOR));

    std::println("OpenGL version: {}",
                 (version != nullptr ? version : "unknown"));
    std::println("OpenGL renderer: {}",
                 (renderer != nullptr ? renderer : "unknown"));
    std::println("OpenGL vendor: {}",
                 (vendor != nullptr ? vendor : "unknown"));

    // OPENGL Triangle
    std::string v_shader_src {R"(
#version 450 core
layout (location = 0) in vec3 aPos;

void main() {
  gl_Position = vec4(aPos.xyz, 1.0);
}
)"};

    std::string f_shader_src {R"(
#version 450 core
out vec4 FragColor;

void main()
{
  FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
)"};

    auto vertex_shad =
        opengl::Shader::compile(opengl::ShaderType::Vertex, v_shader_src);

    if (!vertex_shad) {
        std::println("Vertex shader error: {}.", vertex_shad.error());
        return -1;
    }

    auto fragment_shad =
        opengl::Shader::compile(opengl::ShaderType::Fragment, f_shader_src);

    if (!fragment_shad) {
        std::println("Fragment shader error: {}.", fragment_shad.error());
        return -1;
    }

    auto shader_program = opengl::ShaderProgram::link(vertex_shad.value(),
                                                      fragment_shad.value());

    if (!shader_program) {
        std::println("ShaderProgram error: {}.", shader_program.error());
        return -1;
    }

    opengl::Buffer<VertexData> vbo;
    opengl::Buffer<unsigned int> ebo;

    {

        std::array<VertexData, 4> vertices {
            VertexData{0.5f,  0.5f, 0.0f},
            VertexData{ 0.5f, -0.5f, 0.0f},
            VertexData{-0.5f, -0.5f, 0.0f},
            VertexData{-0.5f,  0.5f, 0.0f}
        };

        std::array<unsigned int, 6> indices {
            0, 1, 3,
            1, 2, 3
        };

        vbo = opengl::Buffer{
            opengl::BufferType::Vertex,
            std::span<VertexData>(vertices)
        };

        ebo = opengl::Buffer{
            opengl::BufferType::Element,
            std::span<unsigned int>(indices)
        };

    }

    opengl::VertexArray vao {std::move(vbo), std::move(ebo)};

    vao.add_attribute(3, GL_FLOAT, opengl::NormalizationType::Normalized, 0);

    // OPENGL end

    bool running {true};
    bool wireframe {false};

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_Z) {
                    wireframe = !wireframe;
                }
            }
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader_program->use();
        vao.bind();

        glPolygonMode(GL_FRONT_AND_BACK,
                      wireframe ? GL_LINE : GL_FILL);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
