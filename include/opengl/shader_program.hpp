#pragma once

#include <cstdint>
#include <expected>
#include <string>
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

    std::uint32_t get_id() const;

private:
    std::uint32_t m_id {0};

    ShaderProgram(std::uint32_t id);
    void destroy();
};

}
