#include <utility>

#include <glad/glad.h>

#include "opengl/shader.hpp"
#include "utility.hpp"

namespace opengl {

Shader::~Shader() {
    destroy();
}

Shader::Shader(std::uint32_t id) : m_id(id)
{}

Shader::Shader(Shader&& other)
    : m_id(std::exchange(other.m_id, 0))
{}

Shader&
Shader::operator=(Shader&& other) {
    if (this != &other) {
        destroy();
        m_id = std::exchange(other.m_id, 0);
    }

    return *this;
}

void
Shader::destroy() {
    if (m_id != 0) {
        glDeleteShader(m_id);
    }

    m_id = 0;
}

std::expected<Shader, std::string>
Shader::compile(ShaderType type, std::string_view source) {
    const auto gl_type = [&](){
        switch(type) {
        case ShaderType::Vertex:   return GL_VERTEX_SHADER;
        case ShaderType::Fragment: return GL_FRAGMENT_SHADER;
        case ShaderType::Geometry: return GL_GEOMETRY_SHADER;
        case ShaderType::Compute:  return GL_COMPUTE_SHADER;
        }

        std::unreachable();
    }();

    std::uint32_t id = glCreateShader(gl_type);

    const char* c_src = source.data();
    const std::int32_t src_length = static_cast<std::int32_t>(source.size());

    glShaderSource(id, 1, &c_src, &src_length);
    glCompileShader(id);

    std::int32_t success {0};
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (!success) {
        std::int32_t log_length {0};
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_length);

        std::string log(log_length, '\0');

        glGetShaderInfoLog(id, log_length, nullptr, log.data());

        glDeleteShader(id);

        return std::unexpected(std::move(log));
    }

    return Shader{id};
}

std::expected<Shader, std::string>
Shader::compile_file(ShaderType type,
                     const std::filesystem::path& filepath) {

    auto source = utility::load_file_content(filepath);

    if (!source) {
        return std::unexpected(source.error());
    }

    return compile(type, *source);
}

std::uint32_t
Shader::get_id() const {
    return m_id;
}

}
