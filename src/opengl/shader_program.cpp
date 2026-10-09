#include <utility>
#include <print>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

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

void
ShaderProgram::set_float(const std::string& name, float value) const {
    const int loc = get_uniform_location(name);

    glUniform1f(loc, value);
}

void
ShaderProgram::set_vec3(const std::string& name, const glm::vec3& vec) const {
    const int loc = get_uniform_location(name);

    glUniform3fv(loc, 1, glm::value_ptr(vec));
}

void
ShaderProgram::set_mat3(const std::string& name, const glm::mat3& mat) const {
    const int loc = get_uniform_location(name);

    glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

void
ShaderProgram::set_mat4(const std::string& name, const glm::mat4& mat) const {
    const int loc = get_uniform_location(name);

    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
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

int
ShaderProgram::get_uniform_location(const std::string& name) const {
    int loc = glGetUniformLocation(m_id, name.c_str());

    if (loc == -1) {
        std::println("Error: Name {} does not correspond to "
                     "an active uniform variable.", name);
    }

    return loc;
}

ShaderProgram::ShaderProgram(std::uint32_t id)
    : m_id(id)
{}

}
