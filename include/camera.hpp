#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 2.0f), float speed = 2.5f);

    glm::mat4 get_view_mat() const;
    void rotate(float delta_x, float delta_y);
    void move(float delta_time, glm::vec3 direction);

    const glm::vec3& get_front() const;
    const glm::vec3& get_right() const;
    const glm::vec3& get_up() const;

    void set_move_speed(float new_speed);
    void set_mouse_sensitivity(float new_sensitivity);

private:
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_right;
    glm::vec3 m_up;
    glm::vec3 m_world_up;

    float m_move_speed;

    float m_yaw {-90.0f};
    float m_pitch {0.0f};
    float m_mouse_sensitivity {0.1f};

    void update_vectors();
};
