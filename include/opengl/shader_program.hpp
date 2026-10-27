#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <span>

#include <glm/glm.hpp>

#include "opengl/shader.hpp"

namespace opengl {

class ShaderProgram {
public:
    ~ShaderProgram();

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    ShaderProgram(ShaderProgram&& other);
    ShaderProgram& operator=(ShaderProgram&& other);

    static std::expected<ShaderProgram, std::string>
    link(const Shader& vertex, const Shader& fragment);

    void use() const;

    void set_int(const std::string& name, int value) const;
    void set_float(const std::string& name, float value) const;
    void set_vec3(const std::string& name, const glm::vec3& vec) const;
    void set_vec4_array(const std::string& name,
                        const std::span<const glm::vec4> data) const;
    void set_mat3(const std::string& name, const glm::mat3& mat) const;
    void set_mat4(const std::string& name, const glm::mat4& mat) const;

    std::uint32_t get_id() const;

private:
    std::uint32_t m_id {0};

    ShaderProgram(std::uint32_t id);
    void destroy();
    int get_uniform_location(const std::string& name) const;
};

}
