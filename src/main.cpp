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

    SDL_Window* window = SDL_CreateWindow("3dmg - OpenGL 4.5", 1280, 720,
                                          SDL_WINDOW_OPENGL);
    if (window == nullptr) {
        SDL_Quit();
        return 1;
    }

    if (!SDL_SetWindowRelativeMouseMode(window, true)) {
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

    Terrain terrain = Terrain::generate(
        1998,

        Transform{
            .position{0.0f, 0.0f, 0.0f},
            .rotation{0.0f, 0.0f, 0.0f},
            .scale{0.0f, 0.0f, 0.0f}
        },

        TerrainConfig{
            .resolution{512},
            .terrain_size{100.0f},
            .height_scale{30.0f},
        },

        generation::PerlinConfig{
            .scale{0.03f},
            .persistence{0.3f},
            .octaves{9}
        }
    );

    float cam_speed {3.0f};
    Camera cam{glm::vec3(95.0f, 90.0f, -50.0f), cam_speed};
    cam.set_look_direction(glm::vec3{-0.6110979, -0.7205517, 0.3276654});
    glm::vec3 cam_move_dir;

    bool running {true};
    bool wireframe {false};

    float time_val {0.0f};
    float previous_frame_time {0.0f};
    float delta_time {0.0f};

    float ambient_strength = 0.1f;
    glm::vec3 ambient_color(1.0f, 1.0f, 1.0f);
    glm::vec3 ambient = ambient_strength * ambient_color;
    glm::vec3 light_dir = glm::normalize(glm::vec3(-5.0f, -8.0f, -2.0f));

    glm::vec3 cube_pos(0.0f, 0.0f, 0.0f);
    glm::vec3 rot_axis(0.0f, 1.0f, 0.0f);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                            1280.0f / 720.0f,
                                            0.1f, 200.0f);
    model = glm::translate(model, cube_pos);

    glm::mat3 normal_mat = glm::transpose(glm::inverse(glm::mat3(model)));

    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_Z) {
                    wireframe = !wireframe;
                }
                else if (event.key.key == SDLK_C) {
                    auto cpos = cam.get_position();
                    auto cfront = cam.get_front();
                    std::println("Cam pos: {}, {}, {}",
                                 cpos.x, cpos.y, cpos.z);
                    std::println("Cam speed: {}", cam_speed);
                    std::println("Cam front: {}, {}, {}",
                                 cfront.x, cfront.y, cfront.z);
                }
            }
            else if (event.type == SDL_EVENT_MOUSE_MOTION) {
                cam.rotate(event.motion.xrel, event.motion.yrel);
            }
            else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                if (event.wheel.y > 0) {
                    cam_speed += 0.5f;
                    cam.set_move_speed(cam_speed);
                }
                else if (event.wheel.y < 0) {
                    cam_speed -= 0.5f;
                    cam.set_move_speed(cam_speed);
                }
            }
        }

        const bool* keys = SDL_GetKeyboardState(nullptr);
        cam_move_dir = {0.0f, 0.0f, 0.0f};

        if (keys[SDL_SCANCODE_W]) {
            cam_move_dir += cam.get_front();
        }
        if (keys[SDL_SCANCODE_S]) {
            cam_move_dir -= cam.get_front();
        }
        if (keys[SDL_SCANCODE_A]) {
            cam_move_dir -= cam.get_right();
        }
        if (keys[SDL_SCANCODE_D]) {
            cam_move_dir += cam.get_right();
        }
        if (keys[SDL_SCANCODE_SPACE]) {
            cam_move_dir += cam.get_up();
        }
        if (keys[SDL_SCANCODE_LCTRL]) {
            cam_move_dir -= cam.get_up();
        }

        time_val = SDL_GetTicks() / 1000.0f;
        delta_time = time_val - previous_frame_time;
        previous_frame_time = time_val;

        cam.move(delta_time, cam_move_dir);

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
        shader_program->set_vec3("object_col", glm::vec3(0.5f, 0.8f, 0.3f));

        terrain.bind_mesh();

        glPolygonMode(GL_FRONT_AND_BACK,
                      wireframe ? GL_LINE : GL_FILL);
        glDrawElements(GL_TRIANGLES, terrain.indices_count(), GL_UNSIGNED_INT, 0);

        terrain.unbind_mesh();

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
