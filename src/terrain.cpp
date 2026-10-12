#include <span>
#include <utility>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "terrain.hpp"
#include "generation/perlin_noise.hpp"

Terrain
Terrain::generate(Transform transform) {
    generation::PerlinNoiseGenerator perlin(1);

    constexpr unsigned int resolution = 128;
    constexpr float terrain_size = 100.0f;

    constexpr int octaves = 4;
    constexpr float persistence = 0.5f;

    constexpr float noise_scale = 0.05f;
    constexpr float height_scale = 15.0f;

    opengl::Buffer<VertexData> vbo;
    opengl::Buffer<unsigned int> ebo;

    std::vector<VertexData> vertices;
    std::vector<unsigned int> indices;

    constexpr unsigned int row_size = resolution + 1;

    vertices.reserve(row_size * row_size);
    indices.reserve(resolution * resolution * 6);

    // VERTICES
    for (unsigned int z = 0; z <= resolution; ++z) {
        for (unsigned int x = 0; x <= resolution; ++x) {

            const float world_x =
                (static_cast<float>(x) / resolution - 0.5f)
                * terrain_size;

            const float world_z =
                (static_cast<float>(z) / resolution - 0.5f)
                * terrain_size;

            const float noise = perlin.octave_perlin2D(
                world_x * noise_scale,
                world_z * noise_scale,
                octaves,
                persistence
            );

            const float height = noise * height_scale;

            vertices.emplace_back(glm::vec3{world_x, height, world_z},
                                  glm::vec3{0.0f, 1.0f, 0.0f});
        }
    }

    // NORMALS
    const auto vertex_index =
        [row_size](unsigned int x, unsigned int z) -> unsigned int {
            return z * row_size + x;
        };

    for (unsigned int z = 0; z <= resolution; ++z) {
        for (unsigned int x = 0; x <= resolution; ++x) {
            const unsigned int left_x = (x > 0) ? x - 1 : x;
            const unsigned int right_x = (x < resolution) ? x + 1 : x;
            const unsigned int down_z = (z > 0) ? z - 1 : z;
            const unsigned int up_z = (z < resolution) ? z + 1 : z;

            const glm::vec3& left =
                vertices[vertex_index(left_x, z)].position;

            const glm::vec3& right =
                vertices[vertex_index(right_x, z)].position;

            const glm::vec3& down =
                vertices[vertex_index(x, down_z)].position;

            const glm::vec3& up =
                vertices[vertex_index(x, up_z)].position;

            const glm::vec3 tangent_x = right - left;
            const glm::vec3 tangent_z = up - down;

            glm::vec3 normal = glm::cross(
                tangent_z,
                tangent_x
            );

            const float length_squared =
                glm::dot(normal, normal);

            if (length_squared > 0.0f) {
                normal *= glm::inversesqrt(length_squared);
            } else {
                normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }

            vertices[vertex_index(x, z)].normal = normal;
        }
    }

    // INDICES
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
                 Transform transform)
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
