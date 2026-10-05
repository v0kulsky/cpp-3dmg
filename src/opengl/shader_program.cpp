#include <utility>

#include <glad/glad.h>

#include "opengl/shader_program.hpp"

namespace opengl {

ShaderProgram::~ShaderProgram() {
    destroy();
}

ShaderProgram::ShaderProgram(ShaderProgram&& other)
    : m_id(std::exchange(other.m_id, 0))
{}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) {
    if (this != &other) {
        destroy();
        m_id = std::exchange(other.m_id, 0);
    }

    return *this;
}

std::expected<ShaderProgram, std::string>
ShaderProgram::link(const Shader& vertex, const Shader& fragment) {
    std::uint32_t id = glCreateProgram();

    glAttachShader(id, vertex.get_id());
    glAttachShader(id, fragment.get_id());

    glLinkProgram(id);

    std::int32_t success {0};
    glGetProgramiv(id, GL_LINK_STATUS, &success);

    if (!success) {
        std::int32_t log_length {0};
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &log_length);

        std::string log(log_length, '\0');

        glGetShaderInfoLog(id, log_length, nullptr, log.data());

        glDeleteProgram(id);

        return std::unexpected(std::move(log));
    }

    return ShaderProgram{id};
}

void
ShaderProgram::use() const {
    glUseProgram(m_id);
}

std::uint32_t
ShaderProgram::get_id() const {
    return m_id;
}

void
ShaderProgram::destroy() {
    if (m_id != 0) {
        glDeleteProgram(m_id);
    }

    m_id = 0;
}

ShaderProgram::ShaderProgram(std::uint32_t id)
    : m_id(id)
{}

}
