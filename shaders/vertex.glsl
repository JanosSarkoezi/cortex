#version 330 core

uniform usamplerBuffer raw_data;
uniform mat4 mvp;
uniform int mode_3d;           // 0: Projektion, 1: 3D
uniform int coord_system_to;
uniform int coord_system_from;
uniform int projection_view;   // 0: XY, 1: YZ, 2: ZX
uniform float morph_factor;
uniform float u_point_size;

const float PI = 3.14159265359;

vec3 calculate_pos(int system, uint b1, uint b2, uint b3, float is_3d) {
    float v1 = float(b1) / 255.0;
    float v2 = float(b2) / 255.0;
    float v3 = float(b3) / 255.0;

    float x, y, z;
    if (system == 0) { // KARTESISCH
        x = v1 * 2.0 - 1.0; y = v2 * 2.0 - 1.0; z = v3 * 2.0 - 1.0;
        if (projection_view == 0) z = mix(0.0, z, is_3d);
        else if (projection_view == 1) x = mix(0.0, x, is_3d);
        else y = mix(0.0, y, is_3d);
    }
    else if (system == 1) { // ZYLINDRISCH
        float theta = v1 * 2.0 * PI;
        float r = (projection_view == 1) ? mix(1.0, v2, is_3d) : v2;
        float h = (projection_view == 0) ? mix(0.0, v3 * 2.0 - 1.0, is_3d) : (v3 * 2.0 - 1.0);
        x = r * cos(theta); y = r * sin(theta); z = h;
    }
    else { // SPHÄRISCH
        float theta = v1 * 2.0 * PI;
        float phi = v2 * PI;
        float r = (projection_view == 0) ? mix(1.0, v3, is_3d) : v3;
        x = r * sin(phi) * cos(theta); y = r * sin(phi) * sin(theta); z = r * cos(phi);
    }
    return vec3(x, y, z);
}

void main() {
    int base_idx = gl_VertexID * 3;
    uint b1 = texelFetch(raw_data, base_idx).r;
    uint b2 = texelFetch(raw_data, base_idx + 1).r;
    uint b3 = texelFetch(raw_data, base_idx + 2).r;

    float t = smoothstep(0.0, 1.0, morph_factor);
    // blend_3d sorgt dafür, dass die Punkte flach werden, bevor das System wechselt
    float blend_3d = (mode_3d == 1) ? t : (1.0 - t);

    vec3 pos_from = calculate_pos(coord_system_from, b1, b2, b3, blend_3d);
    vec3 pos_to = calculate_pos(coord_system_to, b1, b2, b3, blend_3d);

    gl_Position = mvp * vec4(mix(pos_from, pos_to, t), 1.0);
    gl_PointSize = u_point_size;
}
