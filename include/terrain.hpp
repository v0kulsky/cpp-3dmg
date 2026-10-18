#pragma once

#include <glm/glm.hpp>

#include "opengl/vertex_array.hpp"

namespace generation { struct PerlinConfig; }

struct TerrainConfig {
    unsigned int resolution;
    float terrain_size;
    float height_scale;
};

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
    static Terrain
    generate(unsigned int seed, const Transform& transform,
             const TerrainConfig& terrain_config,
             const generation::PerlinConfig& perlin_config);

    Terrain(const Terrain&) = delete;
    Terrain& operator=(const Terrain&) = delete;

    void
    bind_mesh() const;

    void
    unbind_mesh() const;

    const glm::mat4&
    get_model_mat() const;

    const glm::mat3&
    get_normal_mat() const;

    unsigned int
    indices_count() const;

private:
    opengl::VertexArray<VertexData, unsigned int> m_mesh;
    glm::mat4 m_model_matrix;
    glm::mat3 m_normal_matrix;

    Terrain(opengl::VertexArray<VertexData, unsigned int>&& mesh,
            const Transform& transform);

    static std::vector<VertexData>
    generate_vertices(unsigned int seed, unsigned int resolution,
                      float terrain_size, float height_scale,
                      const generation::PerlinConfig& perlin_config);

    static void
    calculate_vertices_normals(unsigned int resolution,
                               std::vector<VertexData>& vertices);

    static std::vector<unsigned int>
    generate_indices(unsigned int resolution);
};
