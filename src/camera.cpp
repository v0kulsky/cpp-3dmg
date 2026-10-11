#include "camera.hpp"

#include <utility>

Camera::Camera(glm::vec3 pos, float speed)
    : m_position(pos), m_front(0.0f, 0.0f, -1.0f), m_world_up(0.0f, 1.0f, 0.0f),
      m_move_speed(speed)
{
    m_right = glm::normalize(glm::cross(m_front, m_world_up));
    m_up = glm::normalize(glm::cross(m_right, m_front));

    update_vectors();
}

glm::mat4 Camera::get_view_mat() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

void Camera::rotate(float delta_x, float delta_y) {
    m_yaw += delta_x * m_mouse_sensitivity;
    m_pitch -= delta_y * m_mouse_sensitivity;

    m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);

    update_vectors();
}

void Camera::move(float delta_time, glm::vec3 direction) {
    m_position = direction * m_move_speed * delta_time + m_position;
}

const glm::vec3& Camera::get_front() const {
    return m_front;
}

const glm::vec3& Camera::get_right() const {
    return m_right;
}

const glm::vec3& Camera::get_up() const {
    return m_up;
}

void Camera::set_move_speed(float new_speed) {
    if (new_speed > 0.0f) {
        m_move_speed = new_speed;
    }
}

void Camera::set_mouse_sensitivity(float new_sensitivity) {
    if (new_sensitivity > 0.0f) {
        m_mouse_sensitivity = new_sensitivity;
    }
}

void Camera::update_vectors() {
    const float yaw = glm::radians(m_yaw);
    const float pitch = glm::radians(m_pitch);

    glm::vec3 front;

    front.x = glm::cos(yaw) * glm::cos(pitch);
    front.y = glm::sin(pitch);
    front.z = glm::sin(yaw) * glm::cos(pitch);

    m_front = glm::normalize(front);

    m_right =
        glm::normalize(glm::cross(m_front, glm::vec3(0.0f, 1.0f, 0.0f)));

    m_up = glm::normalize(glm::cross(m_right, m_front));
}
