#include <print>

#include "ui.hpp"
#include "imgui_stdlib.h"

UI::~UI() {
    shutdown();
}

UI&
UI::instance(SDL_Window* window, SDL_GLContext gl_context, float main_scale) {

    static UI instance(window, gl_context, main_scale);

    return instance;
}

UI&
UI::instance() {
    return instance(nullptr, nullptr, 1.0f);
}

void
UI::shutdown() {
    if (!m_initialized) {
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();

    ImGui::DestroyContext(m_context);

    m_context = nullptr;
    m_initialized = false;
}

void
UI::draw_window(UICtx& context) {
    ImGui::SetNextWindowSizeConstraints(ImVec2(320.0f, 0.0f),
                                        ImVec2(FLT_MAX, FLT_MAX));

    ImGui::Begin("##3dmg_window", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize
                 | ImGuiWindowFlags_NoTitleBar);

    if (ImGui::CollapsingHeader("Terrain generation",
                                ImGuiTreeNodeFlags_None)) {

        ImGui::SeparatorText("Perlin noise configuration");

        ImGui::SliderFloat("Noise scale", &context.perlin_scale,
                           0.001f, 0.075f);

        ImGui::SliderFloat("Persistance", &context.perlin_persistance,
                           0.01f, 0.5f);

        ImGui::SliderInt("Octaves", &context.perlin_octaves, 1, 10);

        ImGui::SeparatorText("Terrain configuration");

        ImGui::SliderFloat("Max height", &context.terrain_max_height,
                           0.0f, 50.0f);

        ImGui::InputScalar("Seed", ImGuiDataType_U32, &context.seed);

        ImGui::Spacing();

        if (ImGui::Button("Generate")) {
            context.generate_terrain = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Random seed")) {
            context.generate_seed = true;
            context.generate_terrain = true;
        }

        ImGui::Spacing();
    }

    if (ImGui::CollapsingHeader("Terrain coloring", ImGuiTreeNodeFlags_None)) {
        ImGui::SeparatorText("Layer coloring");

        if (ImGui::Button("+ Add layer")) {
            context.layers.emplace_back(glm::vec3{1.0f, 1.0f, 1.0f},
                                        std::format("##L{}", m_layer_counter),
                                        std::format("Layer {}", m_layer_counter),
                                        0.0f, true);
            ++m_layer_counter;
        }

        for (auto it = context.layers.begin(); it != context.layers.end();) {

            auto& layer = *it;

            ImGui::InputText(layer.id.c_str(), &layer.name);
            ImGui::SameLine();
            if (ImGui::Button(std::format("Remove{}", layer.id).c_str())) {
                it = context.layers.erase(it);
                continue;
            }
            ImGui::ColorEdit3(std::format("Color{}", layer.id).c_str(),
                              &layer.color.x, ImGuiColorEditFlags_NoInputs);
            ImGui::SameLine();
            ImGui::Checkbox(std::format("Visible{}", layer.id).c_str(),
                            &layer.visible);
            ImGui::SliderFloat(std::format("Upper limit{}", layer.id).c_str(),
                               &layer.high_limit,
                               -context.terrain_max_height,
                               context.terrain_max_height);
            ImGui::Separator();

            ++it;
        }
    }

    ImGui::End();

    ImGui::Render();
}

bool
UI::want_capture_mouse() const {
    return m_io->WantCaptureMouse;
}

void
UI::process_events(const SDL_Event* event) const {
    ImGui_ImplSDL3_ProcessEvent(event);
}

void
UI::new_frame() const {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void
UI::render_drawn_data() const {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

UI::UI(SDL_Window* window, SDL_GLContext gl_context, float main_scale) {

    IMGUI_CHECKVERSION();
    m_context = ImGui::CreateContext();
    m_io = &ImGui::GetIO();
    m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    m_style = &ImGui::GetStyle();
    m_style->ScaleAllSizes(main_scale);
    m_style->FontScaleDpi = main_scale;

    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 450");

    m_initialized = true;
}
