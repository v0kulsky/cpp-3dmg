#include <span>
#include <utility>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "terrain.hpp"
#include "generation/perlin_noise.hpp"

Terrain
Terrain::generate(unsigned int seed, const Transform& transform,
                  const TerrainConfig& terrain_config,
                  const generation::PerlinConfig& perlin_config) {

    opengl::Buffer<VertexData> vbo;
    opengl::Buffer<unsigned int> ebo;

    // VERTICES & NORMALS
    std::vector<VertexData> vertices =
        generate_vertices(seed, terrain_config.resolution,
                          terrain_config.terrain_size,
                          terrain_config.height_scale, perlin_config);

    calculate_vertices_normals(terrain_config.resolution, vertices);

    // INDICES
    std::vector<unsigned int> indices =
        generate_indices(terrain_config.resolution);

    vbo = opengl::Buffer{
        opengl::BufferType::Vertex,
        std::span<VertexData>(vertices)
    };

    ebo = opengl::Buffer{
        opengl::BufferType::Element,
        std::span<unsigned int>(indices)
    };

    opengl::VertexArray mesh{std::move(vbo), std::move(ebo)};
    mesh.add_attribute(3, GL_FLOAT, opengl::NormalizationType::Normalized, 0);
    mesh.add_attribute(3, GL_FLOAT, opengl::NormalizationType::Normalized,
                       offsetof(VertexData, normal));

    return Terrain{std::move(mesh), transform};
}

Terrain::Terrain(opengl::VertexArray<VertexData, unsigned int>&& mesh,
                 const Transform& transform)
    : m_mesh(std::move(mesh)), m_model_matrix(1.0f) {

    m_model_matrix = glm::translate(m_model_matrix, transform.position);

    m_model_matrix = glm::rotate(m_model_matrix,
                                 glm::radians(transform.rotation.x),
                                 glm::vec3(1.0f, 0.0f, 0.0f));

    m_model_matrix = glm::rotate(m_model_matrix,
                                 glm::radians(transform.rotation.y),
                                 glm::vec3(0.0f, 1.0f, 0.0f));

    m_model_matrix = glm::rotate(m_model_matrix,
                                 glm::radians(transform.rotation.z),
                                 glm::vec3(0.0f, 0.0f, 1.0f));

    m_model_matrix = glm::scale(m_model_matrix, transform.scale);
}

void
Terrain::bind_mesh() const {
    m_mesh.bind();
}

void
Terrain::unbind_mesh() const {
    m_mesh.bind();
}

const glm::mat4&
Terrain::get_model_mat() const {
    return m_model_matrix;
}

const glm::mat3&
Terrain::get_normal_mat() const {
    return m_normal_matrix;
}

unsigned int
Terrain::indices_count() const {
    return m_mesh.indices_count();
}

std::vector<VertexData>
Terrain::generate_vertices(unsigned int seed, unsigned int resolution,
                           float terrain_size, float height_scale,
                           const generation::PerlinConfig& perlin_config) {

    generation::PerlinNoiseGenerator perlin(seed);

    std::vector<VertexData> vertices;
    vertices.reserve((resolution + 1) * (resolution + 1));

    for (unsigned int z = 0; z <= resolution; ++z) {
        for (unsigned int x = 0; x <= resolution; ++x) {

            const float world_x =
                (static_cast<float>(x) / resolution - 0.5f)
                * terrain_size;

            const float world_z =
                (static_cast<float>(z) / resolution - 0.5f)
                * terrain_size;

            const float noise =
                perlin.octave_perlin2D(world_x * perlin_config.scale,
                                       world_z * perlin_config.scale,
                                       perlin_config.octaves,
                                       perlin_config.persistence);

            const float height = noise * height_scale;

            vertices.emplace_back(glm::vec3(world_x, height, world_z),
                                  glm::vec3(0.0f, 1.0f, 0.0f));
        }
    }

    return vertices;
}

void
Terrain::calculate_vertices_normals(unsigned int resolution,
                           std::vector<VertexData>& vertices) {

    const auto vertex_index =
        [resolution](unsigned int x, unsigned int z) -> unsigned int {
            return z * (resolution + 1) + x;
        };

    for (unsigned int z = 0; z <= resolution; ++z) {
        for (unsigned int x = 0; x <= resolution; ++x) {

            const unsigned int left_x  = (x > 0) ? x - 1 : x;
            const unsigned int right_x = (x < resolution) ? x + 1 : x;
            const unsigned int down_z  = (z > 0) ? z - 1 : z;
            const unsigned int up_z    = (z < resolution) ? z + 1 : z;

            const glm::vec3& left =
                vertices[vertex_index(left_x, z)].position;

            const glm::vec3& right =
                vertices[vertex_index(right_x, z)].position;

            const glm::vec3& down =
                vertices[vertex_index(x, down_z)].position;

            const glm::vec3& up =
                vertices[vertex_index(x, up_z)].position;

            const float dx = right.x - left.x;
            const float dz = up.z - down.z;

            const float dh_dx =
                (right.y - left.y) / dx;

            const float dh_dz =
                (up.y - down.y) / dz;

            glm::vec3 normal = glm::normalize(glm::vec3(-dh_dx, 1.0f, -dh_dz));

            vertices[vertex_index(x, z)].normal = normal;
        }
    }
}

std::vector<unsigned int>
Terrain::generate_indices(unsigned int resolution) {
    constexpr unsigned int vertices_per_square = 6;

    std::vector<unsigned int> indices;

    indices.reserve(resolution * resolution * vertices_per_square);

    for (unsigned int z = 0; z < resolution; ++z) {
        for (unsigned int x = 0; x < resolution; ++x) {

            const unsigned int top_left =
                z * (resolution + 1) + x;

            const unsigned int top_right =
                top_left + 1;

            const unsigned int bottom_left =
                (z + 1) * (resolution + 1) + x;

            const unsigned int bottom_right =
                bottom_left + 1;

            indices.push_back(top_left);
            indices.push_back(bottom_left);
            indices.push_back(top_right);

            indices.push_back(top_right);
            indices.push_back(bottom_left);
            indices.push_back(bottom_right);
        }
    }

    return indices;
}
