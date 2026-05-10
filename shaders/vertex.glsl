#version 330 core

uniform usamplerBuffer raw_data;
uniform mat4 mvp;
uniform int mode_3d;
uniform int coord_system_from;
uniform int coord_system_to;
uniform float morph_factor;
uniform float u_point_size;

const float PI = 3.14159265359;

vec3 get_pos(int system, uint b1, uint b2, uint b3) {
    float x, y, z;
    if (system == 0) {
        // Kartesisch
        x = float(b1) / 255.0 * 2.0 - 1.0;
        y = float(b2) / 255.0 * 2.0 - 1.0;
        z = (mode_3d != 0) ? float(b3) / 255.0 * 2.0 - 1.0 : 0.0;
    } else if (system == 1) {
        // Zylindrisch
        float theta = float(b1) / 255.0 * 2.0 * PI;
        float r = float(b2) / 255.0;
        x = r * cos(theta);
        y = r * sin(theta);
        z = (mode_3d != 0) ? float(b3) / 255.0 * 2.0 - 1.0 : 0.0;
    } else {
        // Sphärisch
        float theta = float(b1) / 255.0 * 2.0 * PI; // Azimut
        float phi = float(b2) / 255.0 * PI;         // Polar
        float r = (mode_3d != 0) ? float(b3) / 255.0 : 1.0;
        
        x = r * sin(phi) * cos(theta);
        y = r * sin(phi) * sin(theta);
        z = r * cos(phi);
    }
    return vec3(x, y, z);
}

void main() {
    uint b1 = texelFetch(raw_data, gl_VertexID).r;
    uint b2 = texelFetch(raw_data, gl_VertexID + 1).r;
    uint b3 = (mode_3d != 0) ? texelFetch(raw_data, gl_VertexID + 2).r : 0u;

    vec3 pos_from = get_pos(coord_system_from, b1, b2, b3);
    vec3 pos_to = get_pos(coord_system_to, b1, b2, b3);
    
    vec3 final_pos = mix(pos_from, pos_to, smoothstep(0.0, 1.0, morph_factor));

    gl_Position = mvp * vec4(final_pos, 1.0);
    gl_PointSize = u_point_size;
}
