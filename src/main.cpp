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
constexpr float        k_terrain_size       {100.0f};
constexpr float        k_rotation_sensitivity {0.3f};

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

    std::ranges::sort(result, [](const glm::vec4& a, const glm::vec4& b) {
        return a.w < b.w;
    });

    return result;
}

std::optional<opengl::ShaderProgram>
compile_shader_program(const std::filesystem::path& vs_path,
                       const std::filesystem::path& fs_path) {

    auto vs = opengl::Shader::compile_file(opengl::ShaderType::Vertex, vs_path);
    if (!vs) {
        std::println("Vertex shader error: {}.", vs.error());
        return {};
    }

    auto fs =
        opengl::Shader::compile_file(opengl::ShaderType::Fragment, fs_path);

    if (!fs) {
        std::println("Fragment shader error: {}.", fs.error());
        return {};
    }

    auto program = opengl::ShaderProgram::link(vs.value(), fs.value());
    if (!program) {
        std::println("ShaderProgram error: {}.", program.error());
        return {};
    }

    return std::move(program.value());
}

struct AppContext {
    SDL_Window*   window     {nullptr};
    SDL_GLContext gl_context {nullptr};
};

std::optional<AppContext> init_app() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return {};
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window* window = SDL_CreateWindow("3dmg - OpenGL 4.5", 1280, 720,
                                          SDL_WINDOW_OPENGL);
    if (window == nullptr) {
        SDL_Quit();
        return {};
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return {};
    }

    if (!gladLoadGL()) {
        SDL_GL_DestroyContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return {};
    }

    if (!SDL_GL_MakeCurrent(window, gl_context)) {
        SDL_GL_DestroyContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return {};
    }

    return AppContext{window, gl_context};
}

void configure_gl_state() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void print_gl_info() {
    const auto gl_string = [](GLenum name) -> const char* {
        const auto* s = reinterpret_cast<const char*>(glGetString(name));
        return s != nullptr ? s : "unknown";
    };

    std::println("OpenGL version: {}",  gl_string(GL_VERSION));
    std::println("OpenGL renderer: {}", gl_string(GL_RENDERER));
    std::println("OpenGL vendor: {}",   gl_string(GL_VENDOR));
}

Terrain make_terrain(const UICtx& ctx) {
    return Terrain::generate(
        ctx.seed,

        Transform{
            .position {0.0f, 0.0f, 0.0f},
            .rotation {0.0f, 0.0f, 0.0f},
            .scale    {0.0f, 0.0f, 0.0f}
        },

        TerrainConfig{
            .resolution   {k_terrain_resolution},
            .terrain_size {k_terrain_size},
            .height_scale {ctx.terrain_max_height}
        },

        generation::PerlinConfig{
            .scale       {ctx.perlin_scale},
            .persistence {ctx.perlin_persistance},
            .octaves     {ctx.perlin_octaves}
        }
    );
}

struct WaterMesh {
    opengl::VertexArray<VertexData, unsigned int> vao;

    WaterMesh() : vao{build_vao()} {}

    static constexpr int k_index_count = 6;

private:
    static opengl::VertexArray<VertexData, unsigned int> build_vao() {
        std::array<VertexData, 4> quad {
            VertexData {{-50.0f, 0.0f, -50.0f}, {0.0f, 1.0f, 0.0f}},
            VertexData {{-50.0f, 0.0f,  50.0f}, {0.0f, 1.0f, 0.0f}},
            VertexData {{ 50.0f, 0.0f, -50.0f}, {0.0f, 1.0f, 0.0f}},
            VertexData {{ 50.0f, 0.0f,  50.0f}, {0.0f, 1.0f, 0.0f}}
        };

        std::array<unsigned int, 6> indices { 0, 1, 2, 2, 1, 3 };

        opengl::VertexArray<VertexData, unsigned int> vao {
            opengl::Buffer<VertexData> {
                opengl::BufferType::Vertex,
                std::span<VertexData, 4>(quad)
            },
            opengl::Buffer<unsigned int> {
                opengl::BufferType::Element,
                std::span<unsigned int, 6>(indices)
            }
        };

        vao.add_attribute(3, GL_FLOAT,
                          opengl::NormalizationType::Normalized, 0);
        vao.add_attribute(3, GL_FLOAT, opengl::NormalizationType::Normalized,
                          offsetof(VertexData, normal));

        return vao;
    }
};

struct LightingUniforms {
    glm::vec3 ambient  {0.1f, 0.1f, 0.1f};
    glm::vec3 light_dir {glm::normalize(glm::vec3(-5.0f, -8.0f, -2.0f))};
    glm::vec3 light_col {1.0f, 1.0f, 1.0f};
};

void set_lighting_uniforms(opengl::ShaderProgram& program,
                           const LightingUniforms& lighting) {
    program.set_vec3("ambient",   lighting.ambient);
    program.set_vec3("light_dir", lighting.light_dir);
    program.set_vec3("light_col", lighting.light_col);
}

int main() {
    auto app = init_app();
    if (!app) {
        return 1;
    }

    configure_gl_state();
    print_gl_info();

    const float main_scale =
        SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

    UI& ui = UI::instance(app->window, app->gl_context, main_scale);

    // Shader programs
    auto terrain_program = compile_shader_program("./shaders/terrain_v.glsl",
                                                  "./shaders/terrain_f.glsl");
    auto water_program   = compile_shader_program("./shaders/water_v.glsl",
                                                  "./shaders/water_f.glsl");
    if (!terrain_program || !water_program) {
        return 1;
    }

    // Scene state
    UICtx ui_ctx {
        .layers              {},
        .terrain_max_height  {30.0f},
        .seed                {1998},
        .perlin_scale        {0.03f},
        .perlin_persistance  {0.3f},
        .perlin_octaves      {9},
        .water_height        {2.0f},
        .generate_terrain    {false},
        .generate_seed       {false},
        .apply_coloring      {false}
    };

    Terrain terrain = make_terrain(ui_ctx);
    WaterMesh water;

    Camera cam{glm::vec3(95.0f, 90.0f, -50.0f)};
    cam.set_look_direction(glm::vec3{-0.6110979f, -0.7205517f, 0.3276654f});

    const glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                                   1280.0f / 720.0f,
                                                   0.1f, 200.0f);

    const LightingUniforms lighting;

    std::vector<glm::vec4> gpu_coloring_layers;
    int gpu_coloring_layers_count {0};

    float board_rotation {0.0f};
    bool  is_dragging    {false};

    // Main loop
    bool running {true};
    while (running) {
        // Event handling
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ui.process_events(&event);

            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        is_dragging = !ui.want_capture_mouse();
                    }
                    break;

                case SDL_EVENT_MOUSE_BUTTON_UP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        is_dragging = false;
                    }
                    break;

                case SDL_EVENT_MOUSE_MOTION:
                    if (is_dragging) {
                        board_rotation +=
                            event.motion.xrel * k_rotation_sensitivity;

                        board_rotation =
                            std::clamp(board_rotation, -179.0f, 179.0f);
                    }
                    break;

                default:
                    break;
            }
        }

        // UI update
        ui.new_frame();
        ui.draw_window(ui_ctx);

        // Scene mutations triggered by the UI
        if (ui_ctx.generate_seed) {
            ui_ctx.seed          = random_seed();
            ui_ctx.generate_seed = false;
        }

        if (ui_ctx.generate_terrain) {
            terrain                 = make_terrain(ui_ctx);
            ui_ctx.generate_terrain = false;
        }

        if (ui_ctx.apply_coloring) {
            gpu_coloring_layers       = get_gpu_coloring_data(ui_ctx.layers);
            gpu_coloring_layers_count =
                static_cast<int>(gpu_coloring_layers.size());
        }

        // Transform matrices
        const glm::mat4 board_rot = glm::rotate(glm::mat4{1.0f},
                                                 glm::radians(board_rotation),
                                                 glm::vec3{0.0f, 1.0f, 0.0f});

        const glm::mat4 terrain_model  = board_rot;
        const glm::mat3 terrain_normal =
            glm::transpose(glm::inverse(glm::mat3(terrain_model)));

        const glm::mat4 water_model  =
            glm::translate(glm::mat4{1.0f},
                           glm::vec3{0.0f, ui_ctx.water_height, 0.0f})
            * board_rot;

        const glm::mat3 water_normal =
            glm::transpose(glm::inverse(glm::mat3(water_model)));

        // Clear
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Terrain pass
        terrain_program->use();
        terrain_program->set_mat4("model",        terrain_model);
        terrain_program->set_mat4("view",         cam.get_view_mat());
        terrain_program->set_mat4("projection",   projection);
        terrain_program->set_mat3("normal_mat",   terrain_normal);
        terrain_program->set_float("water_height", ui_ctx.water_height);
        set_lighting_uniforms(*terrain_program, lighting);

        if (ui_ctx.apply_coloring) {
            terrain_program->set_vec4_array("coloring_layers",
                                            gpu_coloring_layers);

            terrain_program->set_int("coloring_layers_count",
                                     gpu_coloring_layers_count);

            ui_ctx.apply_coloring = false;
        }

        terrain.bind_mesh();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLES, terrain.indices_count(),
                       GL_UNSIGNED_INT, 0);
        terrain.unbind_mesh();

        // Water pass
        water_program->use();
        water_program->set_mat4("model",      water_model);
        water_program->set_mat4("view",       cam.get_view_mat());
        water_program->set_mat4("projection", projection);
        water_program->set_mat3("normal_mat", water_normal);
        set_lighting_uniforms(*water_program, lighting);

        water.vao.bind();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLES, WaterMesh::k_index_count,
                       GL_UNSIGNED_INT, 0);
        water.vao.unbind();

        ui.render_drawn_data();
        SDL_GL_SwapWindow(app->window);
    }

    ui.shutdown();
    SDL_GL_DestroyContext(app->gl_context);
    SDL_DestroyWindow(app->window);
    SDL_Quit();

    return 0;
}
