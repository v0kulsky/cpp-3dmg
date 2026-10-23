#include <print>
#include <cstddef>

#include <SDL3/SDL.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "generation/perlin_noise.hpp"
#include "opengl/shader_program.hpp"
#include "opengl/vertex_array.hpp"
#include "camera.hpp"
#include "terrain.hpp"
#include "ui.hpp"

std::uint32_t random_seed() {
    static std::mt19937 rng{std::random_device{}()};
    return rng();
}

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

    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

    SDL_Window* window = SDL_CreateWindow("3dmg - OpenGL 4.5", 1280, 720,
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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    UI& ui = UI::instance(window, gl_context, main_scale);

    const std::filesystem::path vertex_shad_location =
        "./shaders/vertex.glsl";

    const std::filesystem::path fragment_shad_location =
        "./shaders/fragment.glsl";

    auto vertex_shad =
        opengl::Shader::compile_file(opengl::ShaderType::Vertex,
                                     vertex_shad_location);

    if (!vertex_shad) {
        std::println("Vertex shader error: {}.", vertex_shad.error());
        return -1;
    }

    auto fragment_shad =
        opengl::Shader::compile_file(opengl::ShaderType::Fragment,
                                     fragment_shad_location);

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

    UICtx app_context {
        .layers {},
        .terrain_max_height {30.0f},

        .seed {1998},
        .perlin_scale {0.03f},
        .perlin_persistance {0.3f},
        .perlin_octaves {9},

        .generate_terrain {false},
        .generate_seed {false}
    };

    Terrain terrain = Terrain::generate(
        app_context.seed,

        Transform{
            .position {0.0f, 0.0f, 0.0f},
            .rotation {0.0f, 0.0f, 0.0f},
            .scale {0.0f, 0.0f, 0.0f}
        },

        TerrainConfig{
            .resolution {512},
            .terrain_size {100.0f},
            .height_scale {app_context.terrain_max_height}
        },

        generation::PerlinConfig{
            .scale {app_context.perlin_scale},
            .persistence {app_context.perlin_persistance},
            .octaves {app_context.perlin_octaves}
        }
    );

    Camera cam{glm::vec3(95.0f, 90.0f, -50.0f)};
    cam.set_look_direction(glm::vec3{-0.6110979, -0.7205517, 0.3276654});

    bool running {true};

    float board_rotation = 0.0f;
    bool is_dragging = false;
    constexpr float rotation_sensitivity = 0.3f;

    float ambient_strength = 0.1f;
    glm::vec3 ambient_color(1.0f, 1.0f, 1.0f);
    glm::vec3 ambient = ambient_strength * ambient_color;
    glm::vec3 light_dir = glm::normalize(glm::vec3(-5.0f, -8.0f, -2.0f));

    glm::vec3 cube_pos(0.0f, 0.0f, 0.0f);
    glm::vec3 rot_axis(0.0f, 1.0f, 0.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                            1280.0f / 720.0f,
                                            0.1f, 200.0f);
    glm::mat4 model;
    glm::mat4 rotation;
    glm::mat3 normal_mat;

    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            ui.process_events(&event);
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    is_dragging = !ui.want_capture_mouse();
                }
            }
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    is_dragging = false;
                }
            }
            else if (event.type == SDL_EVENT_MOUSE_MOTION) {
                if (is_dragging) {
                    board_rotation += event.motion.xrel * rotation_sensitivity;
                    board_rotation = std::clamp(board_rotation, -179.0f, 179.0f);
                }
            }
        }

        ui.new_frame();
        ui.draw_window(app_context);

        if (app_context.generate_seed) {
            app_context.seed = random_seed();
            app_context.generate_seed = false;
        }

        if (app_context.generate_terrain) {
            terrain = Terrain::generate(
                app_context.seed,

                Transform{
                    .position {0.0f, 0.0f, 0.0f},
                    .rotation {0.0f, 0.0f, 0.0f},
                    .scale {0.0f, 0.0f, 0.0f}
                },

                TerrainConfig{
                    .resolution {512},
                    .terrain_size {100.0f},
                    .height_scale {app_context.terrain_max_height},
                },

                generation::PerlinConfig{
                    .scale {app_context.perlin_scale},
                    .persistence {app_context.perlin_persistance},
                    .octaves {app_context.perlin_octaves}
                }
            );

            app_context.generate_terrain = false;
        }

        rotation = glm::rotate(glm::mat4{1.0f}, glm::radians(board_rotation),
                               glm::vec3{0.0f, 1.0f, 0.0f});
        model = glm::translate(glm::mat4{1.0f}, cube_pos) * rotation;
        normal_mat = glm::transpose(glm::inverse(glm::mat3(model)));

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader_program->use();
        shader_program->set_mat4("model", model);
        shader_program->set_mat4("view", cam.get_view_mat());
        shader_program->set_mat4("projection", projection);
        shader_program->set_mat3("normal_mat", normal_mat);

        shader_program->set_vec3("ambient", ambient);
        shader_program->set_vec3("light_dir", light_dir);
        shader_program->set_vec3("light_col", glm::vec3(1.0f, 1.0f, 1.0f));
        // shader_program->set_vec3("object_col", glm::vec3(0.5f, 0.8f, 0.3f));

        terrain.bind_mesh();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLES, terrain.indices_count(), GL_UNSIGNED_INT, 0);

        terrain.unbind_mesh();

        ui.render_drawn_data();
        SDL_GL_SwapWindow(window);
    }

    ui.shutdown();

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
