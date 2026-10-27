#version 450 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;

out vec3 frag_pos;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normal_mat;

void main() {
    gl_Position = projection * view * model * vec4(a_pos, 1.0);

    frag_pos = vec3(model * vec4(a_pos, 1.0f));
    normal = normal_mat * a_normal;
}
