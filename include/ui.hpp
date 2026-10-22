#pragma once

#include <cstdint>
#include <SDL3/SDL.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

struct UICtx {
    float terrain_max_height;
    std::uint32_t seed;
    float perlin_scale;
    float perlin_persistance;
    int perlin_octaves;
    bool generate_terrain;
    bool generate_seed;
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

    [[nodiscard]]
    UICtx
    draw_window(UICtx context) const;

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

    UI(SDL_Window* window, SDL_GLContext gl_context, float main_scale);
};
