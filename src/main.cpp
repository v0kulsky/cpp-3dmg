#include <print>

#include <SDL3/SDL.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window* window = SDL_CreateWindow("3dmq - OpenGL 4.2", 1280, 720,
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

    // TEST GLM
    glm::vec3 position{1.0f, 2.0f, 3.0f};

    glm::mat4 model =
        glm::translate(glm::mat4{1.0f}, position);

    std::println("GLM position: ({}, {}, {})",
                 position.x,
                 position.y,
                 position.z);

    std::println("GLM model[3]: ({}, {}, {}, {})",
                 model[3].x,
                 model[3].y,
                 model[3].z,
                 model[3].w);
    // END TEST GLM

    bool running = true;

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
