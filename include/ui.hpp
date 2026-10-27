#pragma once

#include <cstdint>
#include <string>
#include <SDL3/SDL.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

#include <glm/glm.hpp>

struct Layer {
    glm::vec3 color;
    std::string id;
    std::string name;
    float high_limit;
    bool visible;
};

struct UICtx {
    std::vector<Layer> layers;
    float terrain_max_height;
    std::uint32_t seed;
    float perlin_scale;
    float perlin_persistance;
    int perlin_octaves;
    float water_height;
    bool generate_terrain;
    bool generate_seed;
    bool apply_coloring;
};

class UI {
public:
    UI(const UI&) = delete;
    UI(const UI&&) = delete;

    UI& operator=(const UI&) = delete;
    UI& operator=(const UI&&) = delete;

    ~UI();

    static UI&
    instance(SDL_Window* window, SDL_GLContext gl_context, float main_scale);

    static UI&
    instance();

    void
    shutdown();

    void
    draw_window(UICtx& context);

    bool
    want_capture_mouse() const;

    void
    process_events(const SDL_Event* event) const;

    void
    new_frame() const;

    void
    render_drawn_data() const;

private:
    ImGuiContext* m_context {nullptr};
    ImGuiStyle* m_style {nullptr};
    ImGuiIO* m_io {nullptr};
    bool m_initialized {false};

    std::uint32_t m_layer_counter {0};

    UI(SDL_Window* window, SDL_GLContext gl_context, float main_scale);
};
