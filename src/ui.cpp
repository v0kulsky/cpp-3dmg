#include "ui.hpp"

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

[[nodiscard]]
UICtx
UI::draw_window(UICtx context) const {
    ImGui::Begin("Terrain generation", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::SeparatorText("Perlin noise configuration");
    ImGui::SliderFloat("Noise scale", &context.perlin_scale, 0.001f, 0.075f);
    ImGui::SliderFloat("Persistance", &context.perlin_persistance, 0.01f, 0.5f);
    ImGui::SliderInt("Octaves", &context.perlin_octaves, 1, 10);

    ImGui::SeparatorText("Terrain configuration");
    ImGui::SliderFloat("Max height", &context.terrain_max_height, 0.0f, 50.0f);

    ImGui::InputScalar("Seed", ImGuiDataType_U32, &context.seed);
    ImGui::SameLine();
    if (ImGui::Button("Random")) {
        context.generate_seed = true;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Generate!")) {
        context.generate_terrain = true;
    }
    ImGui::End();

    ImGui::Render();
    return context;
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
