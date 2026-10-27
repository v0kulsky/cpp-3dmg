#version 450 core

in vec3 frag_pos;
in vec3 normal;

out vec4 frag_color;

uniform vec3 light_dir;
uniform vec3 ambient;
uniform vec3 light_col;

uniform vec4 coloring_layers[32];
uniform int coloring_layers_count;

uniform float water_height;

void main() {
    vec3 base_col = vec3(1.0, 0.5, 0.3);
    float blend_width = 4.0f;

    if (coloring_layers_count > 0) {
        base_col = coloring_layers[coloring_layers_count - 1].xyz;

        for (int i = 0; i < coloring_layers_count; ++i) {
            float upper_limit = coloring_layers[i].w;
            vec3 current_color = coloring_layers[i].xyz;

            if (frag_pos.y <= upper_limit) {
                base_col = current_color;

                if (i > 0 && blend_width > 0.0) {
                    float lower_limit = coloring_layers[i - 1].w;
                    vec3 previous_color = coloring_layers[i - 1].xyz;

                    float blend_start = upper_limit - blend_width;

                    float blend = smoothstep(
                        blend_start,
                        upper_limit,
                        frag_pos.y
                    );

                    base_col = mix(
                        previous_color,
                        current_color,
                        blend
                    );
                }

                break;
            }
        }
    }

    float depth_below_water = water_height - frag_pos.y;
    float wet_factor = smoothstep(0.0, 2.0, depth_below_water);
    vec3 wet_color = base_col * vec3(0.50, 0.42, 0.32);

    base_col = mix(base_col, wet_color, wet_factor);

    vec3 norm = normalize(normal);

    float diff = max(dot(norm, -light_dir), 0.0f);
    vec3 diffuse = diff * light_col;

    vec3 result = (ambient + diffuse) * base_col;

    frag_color = vec4(result, 1.0f);
}
