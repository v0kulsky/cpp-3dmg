#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <utility>
#include <string_view>
#include <filesystem>

namespace opengl {

enum class ShaderType {
    Vertex,
    Fragment,
    Geometry,
    Compute
};

class Shader {
public:
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other);
    Shader& operator=(Shader&& other);

    static std::expected<Shader, std::string>
    compile(ShaderType type, std::string_view source);

    static std::expected<Shader, std::string>
    compile_file(ShaderType type, const std::filesystem::path& filepath);

    std::uint32_t get_id() const;

private:
    std::uint32_t m_id {0};

    Shader(std::uint32_t id);
    void destroy();
};

}
