#pragma once

#include <cstdint>
#include <utility>

#include "opengl/buffer.hpp"

namespace opengl {

enum class NormalizationType {
    Normalized,
    ShouldBeNormalized
};

template<typename vbo_t, typename ebo_t>
class VertexArray {
public:
    VertexArray() = default;
    VertexArray(Buffer<vbo_t>&& vbo, Buffer<ebo_t>&& ebo);

    ~VertexArray();

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    VertexArray(VertexArray&& other);
    VertexArray& operator=(VertexArray&& other);

    void bind() const;
    void unbind() const;

    void add_attribute(std::uint32_t size, std::int32_t type,
                       NormalizationType norm_type, std::size_t offset);

    unsigned int indices_count() const;

private:
    Buffer<vbo_t> m_vbo;
    Buffer<ebo_t> m_ebo;

    std::uint32_t m_id {0};
    std::size_t m_attrib_counter {0};

    void destroy();
};

// IMPLEMENTATION
template<typename vbo_t, typename ebo_t>
VertexArray<vbo_t, ebo_t>::VertexArray(Buffer<vbo_t>&& vbo,
                                       Buffer<ebo_t>&& ebo)
    : m_vbo(std::move(vbo)), m_ebo(std::move(ebo)) {

    glGenVertexArrays(1, &m_id);
}

template<typename vbo_t, typename ebo_t>
VertexArray<vbo_t, ebo_t>::~VertexArray() {
    destroy();
}

template<typename vbo_t, typename ebo_t>
VertexArray<vbo_t, ebo_t>::VertexArray(VertexArray<vbo_t, ebo_t>&& other)
    : m_vbo(std::move(other.m_vbo)),
      m_ebo(std::move(other.m_ebo)),
      m_id(std::exchange(other.m_id, 0)),
      m_attrib_counter(std::exchange(other.m_attrib_counter, 0))
{}

template<typename vbo_t, typename ebo_t>
VertexArray<vbo_t, ebo_t>&
VertexArray<vbo_t, ebo_t>::operator=(VertexArray<vbo_t, ebo_t>&& other) {
    if (this != &other) {
        destroy();

        m_id = std::exchange(other.m_id, 0);
        m_attrib_counter = std::exchange(other.m_attrib_counter, 0);
        m_vbo = std::move(other.m_vbo);
        m_ebo = std::move(other.m_ebo);
    }

    return *this;
}

template<typename vbo_t, typename ebo_t>
void
VertexArray<vbo_t, ebo_t>::bind() const {
    glBindVertexArray(m_id);
    m_ebo.bind();
}

template<typename vbo_t, typename ebo_t>
void
VertexArray<vbo_t, ebo_t>::unbind() const {
    glBindVertexArray(0);
}

template<typename vbo_t, typename ebo_t>
void
VertexArray<vbo_t, ebo_t>::add_attribute(std::uint32_t size, std::int32_t type,
                                         NormalizationType norm_type,
                                         std::size_t offset) {

    auto gl_norm_type = [&](){
        switch(norm_type) {
        case NormalizationType::Normalized: return GL_FALSE;
        case NormalizationType::ShouldBeNormalized: return GL_TRUE;
        }

        std::unreachable();
    }();

    glBindVertexArray(m_id);

    m_vbo.bind();

    glVertexAttribPointer(m_attrib_counter, size, type, gl_norm_type,
                          m_vbo.get_element_size(),
                          reinterpret_cast<void*>(offset));
    glEnableVertexAttribArray(m_attrib_counter);

    m_vbo.unbind();

    m_ebo.bind();

    glBindVertexArray(0);

    ++m_attrib_counter;
}

template<typename vbo_t, typename ebo_t>
unsigned int
VertexArray<vbo_t, ebo_t>::indices_count() const {
    return m_ebo.count();
}

template<typename vbo_t, typename ebo_t>
void
VertexArray<vbo_t, ebo_t>::destroy() {
    if (m_id != 0) {
        glDeleteVertexArrays(1, &m_id);
        m_id = 0;
        m_attrib_counter = 0;
    }
}

}
