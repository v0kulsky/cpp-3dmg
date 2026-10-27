#include <print>
#include <cstddef>
#include <algorithm>
#include <optional>

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

constexpr unsigned int k_terrain_resolution {512};
constexpr float k_terrain_size {100.0f};

using ShaderPair = std::pair<opengl::Shader, opengl::Shader>;

std::uint32_t random_seed() {
    static std::mt19937 rng{std::random_device{}()};
    return rng();
}

std::vector<glm::vec4> get_gpu_coloring_data(std::vector<Layer>& layers) {
    std::vector<glm::vec4> result;
    result.reserve(layers.size());

    for (const Layer& layer : layers) {
        result.emplace_back(layer.color, layer.high_limit);
    }

    std::ranges::sort(result, [](const glm::vec4& a, const glm::vec4& b)
    {
        return a.w < b.w;
    });

    return result;
}

std::optional<ShaderPair>
compile_shader_pair(const std::filesystem::path& vs_path,
                    const std::filesystem::path& fs_path) {

    auto vs =
        opengl::Shader::compile_file(opengl::ShaderType::Vertex, vs_path);

    auto fs =
        opengl::Shader::compile_file(opengl::ShaderType::Fragment, fs_path);

    if (!vs) {
        std::println("Vertex shader error: {}.", vs.error());
        return {};
    }

    if (!fs) {
        std::println("Fragment shader error: {}.", fs.error());
        return {};
    }

    return std::make_pair(std::move(vs.value()), std::move(fs.value()));
}

std::optional<opengl::ShaderProgram>
compile_shader_program(const std::filesystem::path& vs_path,
                       const std::filesystem::path& fs_path) {

    auto shaders = compile_shader_pair(vs_path, fs_path);

    if (!shaders) {
        return {};
    }

    auto shader_program =
        opengl::ShaderProgram::link(shaders.value().first,
                                    shaders.value().second);

    if (!shader_program) {
        std::println("ShaderProgram error: {}.", shader_program.error());
        return {};
    }

    return std::move(shader_program.value());
}

int main() {
    // app initialization
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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // after app init printing
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

    // ui
    UI& ui = UI::instance(window, gl_context, main_scale);

    // variables initialization
    auto terrain_program = compile_shader_program("./shaders/terrain_v.glsl",
                                                  "./shaders/terrain_f.glsl");

    auto water_program = compile_shader_program("./shaders/water_v.glsl",
                                                "./shaders/water_f.glsl");

    if (!terrain_program || !water_program) {
        return -1;
    }

    UICtx app_context {
        .layers {},
        .terrain_max_height {30.0f},

        .seed {1998},
        .perlin_scale {0.03f},
        .perlin_persistance {0.3f},
        .perlin_octaves {9},

        .water_height {2.0f},
        .generate_terrain {false},
        .generate_seed {false},
        .apply_coloring {false}
    };

    Terrain terrain = Terrain::generate(
        app_context.seed,

        Transform{
            .position {0.0f, 0.0f, 0.0f},
            .rotation {0.0f, 0.0f, 0.0f},
            .scale {0.0f, 0.0f, 0.0f}
        },

        TerrainConfig{
            .resolution {k_terrain_resolution},
            .terrain_size {k_terrain_size},
            .height_scale {app_context.terrain_max_height}
        },

        generation::PerlinConfig{
            .scale {app_context.perlin_scale},
            .persistence {app_context.perlin_persistance},
            .octaves {app_context.perlin_octaves}
        }
    );

    std::array water_quad {
        VertexData {{-50.0f, 0.0f, -50.0f}, {0.0f, 1.0f, 0.0f}},
        VertexData {{-50.0f, 0.0f,  50.0f}, {0.0f, 1.0f, 0.0f}},
        VertexData {{ 50.0f, 0.0f, -50.0f}, {0.0f, 1.0f, 0.0f}},
        VertexData {{ 50.0f, 0.0f,  50.0f}, {0.0f, 1.0f, 0.0f}}
    };

    std::array<unsigned int, 6> water_indices {
        0, 1, 2,
        2, 1, 3
    };

    opengl::VertexArray water {
        opengl::Buffer<VertexData> {
            opengl::BufferType::Vertex,
            std::span<VertexData, 4>(water_quad)
        },
        opengl::Buffer<unsigned int> {
            opengl::BufferType::Element,
            std::span<unsigned int, 6>(water_indices)
        }
    };

    water.add_attribute(3, GL_FLOAT, opengl::NormalizationType::Normalized, 0);
    water.add_attribute(3, GL_FLOAT, opengl::NormalizationType::Normalized,
                       offsetof(VertexData, normal));

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

    std::vector<glm::vec4> gpu_coloring_layers;
    int gpu_coloring_layers_count {0};

    // WATER
    glm::mat4 water_model = glm::mat4{1.0f};
    glm::mat4 water_rotation;
    glm::mat3 water_normal_mat;

    SDL_Event event;

    // main loop
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

        if (app_context.apply_coloring) {
            gpu_coloring_layers = get_gpu_coloring_data(app_context.layers);
            gpu_coloring_layers_count = gpu_coloring_layers.size();
        }

        rotation = glm::rotate(glm::mat4{1.0f}, glm::radians(board_rotation),
                               glm::vec3{0.0f, 1.0f, 0.0f});
        model = glm::translate(glm::mat4{1.0f}, cube_pos) * rotation;
        normal_mat = glm::transpose(glm::inverse(glm::mat3(model)));

        water_rotation = glm::rotate(glm::mat4{1.0f},
                                     glm::radians(board_rotation),
                                     glm::vec3{0.0f, 1.0f, 0.0f});
        water_model =
            glm::translate(glm::mat4{1.0f},
                           glm::vec3{0.0f, app_context.water_height, 0.0f})
            * water_rotation;

        water_normal_mat = glm::transpose(glm::inverse(glm::mat3(water_model)));

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // terrain rendering
        terrain_program->use();
        terrain_program->set_mat4("model", model);
        terrain_program->set_mat4("view", cam.get_view_mat());
        terrain_program->set_mat4("projection", projection);
        terrain_program->set_mat3("normal_mat", normal_mat);

        terrain_program->set_vec3("ambient", ambient);
        terrain_program->set_vec3("light_dir", light_dir);
        terrain_program->set_vec3("light_col", glm::vec3(1.0f, 1.0f, 1.0f));
        terrain_program->set_float("water_height", app_context.water_height);

        if (app_context.apply_coloring) {
            terrain_program->set_vec4_array("coloring_layers",
                                           gpu_coloring_layers);
            terrain_program->set_int("coloring_layers_count",
                                    gpu_coloring_layers_count);

            app_context.apply_coloring = false;
        }

        terrain_program->use();
        terrain.bind_mesh();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLES, terrain.indices_count(), GL_UNSIGNED_INT, 0);
        terrain.unbind_mesh();

        // water rendering
        water_program->use();
        water_program->set_mat4("model", water_model);
        water_program->set_mat4("view", cam.get_view_mat());
        water_program->set_mat4("projection", projection);
        water_program->set_mat3("normal_mat", water_normal_mat);

        water_program->set_vec3("ambient", ambient);
        water_program->set_vec3("light_dir", light_dir);
        water_program->set_vec3("light_col", glm::vec3(1.0f, 1.0f, 1.0f));

        water.bind();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        water.unbind();

        ui.render_drawn_data();
        SDL_GL_SwapWindow(window);
    }

    ui.shutdown();

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
