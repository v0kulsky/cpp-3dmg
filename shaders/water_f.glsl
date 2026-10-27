#version 450 core

in vec3 frag_pos;
in vec3 normal;

out vec4 frag_color;

uniform vec3 light_dir;
uniform vec3 ambient;
uniform vec3 light_col;

void main() {
    vec3 base_col = vec3(0.0, 0.3, 0.8);

    vec3 norm = normalize(normal);

    float diff = max(dot(norm, -light_dir), 0.0f);
    vec3 diffuse = diff * light_col;

    vec3 result = (ambient + diffuse) * base_col;

    frag_color = vec4(result, 0.6f);
}
