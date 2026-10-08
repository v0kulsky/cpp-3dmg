#pragma once

#include <cstdint>
#include <utility>

#include <glad/glad.h>

namespace opengl {

enum class BufferType {
    Vertex,
    Element
};

template<typename T>
class Buffer {
public:
    Buffer() = default;
    Buffer(BufferType type, std::span<T> data);
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&& other);
    Buffer& operator=(Buffer&& other);

    void bind() const;
    void unbind() const;

    std::size_t get_element_size() const;

private:
    std::uint32_t m_id {0};
    std::int32_t m_type;
    std::size_t m_element_size {0};

    void destroy();
};

// IMPLEMENTATION
template<typename T>
Buffer<T>::Buffer(BufferType type, std::span<T> data) {
    auto gl_type = [&](){
        switch(type) {
        case BufferType::Vertex: return GL_ARRAY_BUFFER;
        case BufferType::Element: return GL_ELEMENT_ARRAY_BUFFER;
        }

        std::unreachable();
    }();

    glGenBuffers(1, &m_id);
    m_type = static_cast<std::uint32_t>(gl_type);
    m_element_size = sizeof(T);

    bind();
    glBufferData(m_type, data.size() * m_element_size, data.data(),
                 GL_STATIC_DRAW);
    unbind();
}

template<typename T>
Buffer<T>::~Buffer() {
    destroy();
}

template<typename T>
Buffer<T>::Buffer(Buffer<T>&& other)
    : m_id(std::exchange(other.m_id, 0)),
      m_type(std::exchange(other.m_type, 0)),
      m_element_size(std::exchange(other.m_element_size, 0))
{}

template<typename T>
Buffer<T>&
Buffer<T>::operator=(Buffer<T>&& other) {
    if (this != &other) {
        destroy();

        m_id = std::exchange(other.m_id, 0);
        m_type = std::exchange(other.m_type, 0);
        m_element_size = std::exchange(other.m_element_size, 0);
    }

    return *this;
}

template<typename T>
void
Buffer<T>::bind() const {
    glBindBuffer(m_type, m_id);
}

template<typename T>
void
Buffer<T>::unbind() const {
    glBindBuffer(m_type, 0);
}

template<typename T>
std::size_t
Buffer<T>::get_element_size() const {
    return m_element_size;
}

template<typename T>
void
Buffer<T>::destroy() {
    if (m_id != 0) {
        glDeleteBuffers(1, &m_id);
        m_id = 0;
        m_element_size = 0;
    }
}

}
