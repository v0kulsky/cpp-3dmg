#pragma once

#include <glm/glm.hpp>

#include "opengl/vertex_array.hpp"

struct Transform {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

struct VertexData {
    glm::vec3 position;
    glm::vec3 normal;
};

class Terrain {
public:
    static Terrain generate(Transform transform);

    Terrain(const Terrain&) = delete;
    Terrain& operator=(const Terrain&) = delete;

    void bind_mesh() const;
    void unbind_mesh() const;

    const glm::mat4& get_model_mat() const;
    const glm::mat3& get_normal_mat() const;

private:
    opengl::VertexArray<VertexData, unsigned int> m_mesh;
    glm::mat4 m_model_matrix;
    glm::mat3 m_normal_matrix;

    Terrain(opengl::VertexArray<VertexData, unsigned int>&& mesh,
            Transform transform);
};
