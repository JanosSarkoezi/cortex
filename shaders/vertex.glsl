#version 330 core

uniform usamplerBuffer raw_data;
uniform mat4 mvp;
uniform int mode_3d;           // 0: Projektion (2D), 1: 3D Ansicht
uniform int coord_system_to;   // 0: Kartesisch, 1: Zylindrisch, 2: Sphärisch
uniform int coord_system_from; // 0: Kartesisch, 1: Zylindrisch, 2: Sphärisch
uniform int projection_view;   // 0: XY/Ebene 1, 1: YZ/Ebene 2, 2: ZX/Ebene 3
uniform float morph_factor;    // Korrigiert: Muss float sein!
uniform float u_point_size;

const float PI = 3.14159265359;

vec3 get_pos(int system, uint b1, uint b2, uint b3) {
    float x, y, z;
    float v1 = float(b1) / 255.0;
    float v2 = float(b2) / 255.0;
    float v3 = float(b3) / 255.0;

    if (system == 0) { // KARTESISCH
        float cx = v1 * 2.0 - 1.0;
        float cy = v2 * 2.0 - 1.0;
        float cz = v3 * 2.0 - 1.0;

        x = cx; y = cy; z = cz;
    } else if (system == 1) { // ZYLINDRISCH
        float theta = v1 * 2.0 * PI;
        float r = v2;
        float h = v3 * 2.0 - 1.0;

        if (mode_3d == 0) {
            if (projection_view == 0)      { h = 0.0; } // Top-Down Querschnitt
            else if (projection_view == 1) { r = 1.0; } // Seitenwand-Abwicklung
        }
        x = r * cos(theta);
        y = r * sin(theta);
        z = h;

    } else { // SPHÄRISCH
        float theta = v1 * 2.0 * PI;
        float phi = v2 * PI;
        float r = v3;

        if (mode_3d == 0 && projection_view == 0) { r = 1.0; } // Auf Kugeloberfläche

        x = r * sin(phi) * cos(theta);
        y = r * sin(phi) * sin(theta);
        z = r * cos(phi);
    }
    return vec3(x, y, z);
}

void main() {
    uint b1 = texelFetch(raw_data, gl_VertexID).r;
    uint b2 = texelFetch(raw_data, gl_VertexID + 1).r;
    uint b3 = texelFetch(raw_data, gl_VertexID + 2).r;

    vec3 pos_from = get_pos(coord_system_from, b1, b2, b3);
    vec3 pos_to = get_pos(coord_system_to, b1, b2, b3);

    vec3 final_pos = mix(pos_from, pos_to, smoothstep(0.0, 1.0, morph_factor));

    gl_Position = mvp * vec4(final_pos, 1.0);
    gl_PointSize = u_point_size;
}
