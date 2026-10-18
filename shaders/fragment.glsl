#version 450 core

in vec3 frag_pos;
in vec3 normal;

out vec4 frag_color;

uniform vec3 light_dir;
uniform vec3 ambient;
uniform vec3 object_col;
uniform vec3 light_col;

void main()
{
    // diffuse light
    vec3 norm = normalize(normal);
    //vec3 light_dir = normalize(light_pos - frag_pos);

    float diff = max(dot(norm, -light_dir), 0.0f);
    vec3 diffuse = diff * light_col;

    vec3 result = (ambient + diffuse) * object_col;

    frag_color = vec4(result, 1.0f);
}
