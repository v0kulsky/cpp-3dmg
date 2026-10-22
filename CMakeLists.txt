cmake_minimum_required(VERSION 3.28)

project(
  3dmg
  VERSION 0.1
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENTIONS OFF)

set(GLAD_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party/glad)
set(IMGUI_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party/imgui)

find_package(SDL3 CONFIG REQUIRED)

include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
)

FetchContent_MakeAvailable(glm)

add_executable(
  3dmg
  src/main.cpp
  src/generation/perlin_noise.cpp
  src/opengl/shader.cpp
  src/opengl/shader_program.cpp
  src/utility.cpp
  src/camera.cpp
  src/terrain.cpp
  src/ui.cpp
  ${GLAD_DIR}/src/glad.c
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_demo.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/imgui_impl_sdl3.cpp
  ${IMGUI_DIR}/imgui_impl_opengl3.cpp)

set_source_files_properties(
    ${GLAD_DIR}/src/glad.c
    PROPERTIES COMPILE_OPTIONS "-Wno-pedantic"
)

target_compile_options(
  3dmg PRIVATE
  -Wall
  -Wextra
  -Wpedantic
  -Werror)

target_link_libraries(
  3dmg PRIVATE
  SDL3::SDL3
  glm::glm)

target_include_directories(
  3dmg PRIVATE
  include
  ${GLAD_DIR}/include
  ${IMGUI_DIR})
