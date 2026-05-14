#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in float aCount;

uniform usamplerBuffer raw_data;
uniform mat4 mvp;
uniform float u_point_size;

uniform int coord_system_from;
uniform int coord_system_to;
uniform int projection_view;   // 0: XY, 1: YZ, 2: ZX
uniform int u_proj_from;       // 0: 3D, 1: Flat
uniform int u_proj_to;         // 0: 3D, 1: Flat
uniform float morph_factor;    // 0.0 bis 1.0

uniform int u_use_histogram;   // 0: Rohdaten (TBO), 1: Histogramm (VBO)

out float vCount;

const float PI = 3.14159265359;

// --- HILFSFUNKTIONEN FÜR DIE KOORDINATEN ---

vec3 get_3d_pos(int system, float v1, float v2, float v3) {
    if (system == 0) { // KARTESISCH
        return vec3(v1 * 2.0 - 1.0, v2 * 2.0 - 1.0, v3 * 2.0 - 1.0);
    }
    if (system == 1) { // ZYLINDRISCH
        float theta = v1 * 2.0 * PI;
        float r = sqrt(v2);
        return vec3(r * cos(theta), r * sin(theta), v3 * 2.0 - 1.0);
    }
    // SPHÄRISCH
    float theta = v1 * 2.0 * PI;
    float phi   = acos(1.0 - 2.0 * v2);
    float r = pow(v3, 1.0 / 3.0);
    return vec3(r * sin(phi) * cos(theta), r * sin(phi) * sin(theta), r * cos(phi));
}

vec3 get_proj_pos(int system, float v1, float v2, float v3) {
    if (system == 0) { // KARTESISCH
        vec3 p = vec3(v1 * 2.0 - 1.0, v2 * 2.0 - 1.0, v3 * 2.0 - 1.0);
        if (projection_view == 0)      p.z = 0.0;
        else if (projection_view == 1) p.x = 0.0;
        else                           p.y = 0.0;
        return p;
    }
    if (system == 1) { // ZYLINDRISCH
        float theta = v1 * 2.0 * PI;
        float r = (projection_view == 1) ? 1.0 : sqrt(v2);
        float h = (projection_view == 0) ? 0.0 : (v3 * 2.0 - 1.0);
        return vec3(r * cos(theta), r * sin(theta), h);
    }
    // SPHÄRISCH
    float theta = v1 * 2.0 * PI;
    float phi   = (projection_view == 1) ? PI/2.0 : acos(1.0 - 2.0 * v2);
    float r     = (projection_view == 0) ? 1.0 : pow(v3, 1.0 / 3.0);
    return vec3(r * sin(phi) * cos(theta), r * sin(phi) * sin(theta), r * cos(phi));
}

// --- HAUPTPROGRAMM ---

void main() {
    float v1, v2, v3;
    float count;

    if (u_use_histogram == 1) {
        // Daten aus VBO Attributen
        v1 = aPos.x / 255.0;
        v2 = aPos.y / 255.0;
        v3 = aPos.z / 255.0;
        count = aCount;
    } else {
        // Daten-Fetch aus TBO (Sliding Window: i, i+1, i+2)
        int base_idx = gl_VertexID;
        v1 = float(texelFetch(raw_data, base_idx).r) / 255.0;
        v2 = float(texelFetch(raw_data, base_idx + 1).r) / 255.0;
        v3 = float(texelFetch(raw_data, base_idx + 2).r) / 255.0;
        count = 1.0;
    }

    // VCount an Fragment Shader weitergeben (für Helligkeit)
    // Wir nutzen den rohen Count, damit das Akkumulations-Prinzip (log-Mapping) funktioniert
    vCount = (u_use_histogram == 1) ? count : 1.0;

    float t = smoothstep(0.0, 1.0, morph_factor);
    float blend_p = mix(float(u_proj_from), float(u_proj_to), t);

    vec3 pos_from  = mix(get_3d_pos(coord_system_from, v1, v2, v3), get_proj_pos(coord_system_from, v1, v2, v3), blend_p);
    vec3 pos_to    = mix(get_3d_pos(coord_system_to, v1, v2, v3), get_proj_pos(coord_system_to, v1, v2, v3), blend_p);

    gl_Position = mvp * vec4(mix(pos_from, pos_to, t), 1.0);
    gl_PointSize = u_point_size;
}
