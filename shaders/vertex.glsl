#version 330 core

uniform usamplerBuffer raw_data;
uniform mat4 mvp;
uniform float u_point_size;

uniform int coord_system_from;
uniform int coord_system_to;
uniform int projection_view;   // 0: XY, 1: YZ, 2: ZX
uniform int u_is_projected;    // 0: Volle 3D Wolke, 1: Flache Projektion
uniform float morph_factor;    // 0.0 bis 1.0

const float PI = 3.14159265359;

// --- HILFSFUNKTIONEN FÜR DIE KOORDINATEN ---

vec3 get_3d_pos(int system, float v1, float v2, float v3) {
    if (system == 0) { // KARTESISCH
        return vec3(v1 * 2.0 - 1.0, v2 * 2.0 - 1.0, v3 * 2.0 - 1.0);
    }
    if (system == 1) { // ZYLINDRISCH
        float theta = v1 * 2.0 * PI;
        return vec3(v2 * cos(theta), v2 * sin(theta), v3 * 2.0 - 1.0);
    }
    // SPHÄRISCH
    float theta = v1 * 2.0 * PI;
    float phi   = v2 * PI;
    return vec3(v3 * sin(phi) * cos(theta), v3 * sin(phi) * sin(theta), v3 * cos(phi));
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
        float r = (projection_view == 1) ? 1.0 : v2;
        float h = (projection_view == 0) ? 0.0 : (v3 * 2.0 - 1.0);
        return vec3(r * cos(theta), r * sin(theta), h);
    }
    // SPHÄRISCH
    float theta = v1 * 2.0 * PI;
    float phi   = (projection_view == 1) ? PI/2.0 : (v2 * PI);
    float r     = (projection_view == 0) ? 1.0 : v3;
    return vec3(r * sin(phi) * cos(theta), r * sin(phi) * sin(theta), r * cos(phi));
}

// --- HAUPTPROGRAMM ---

void main() {
    // Daten-Fetch (3 Bytes pro Punkt)
    int base_idx = gl_VertexID * 3;
    float v1 = float(texelFetch(raw_data, base_idx).r) / 255.0;
    float v2 = float(texelFetch(raw_data, base_idx + 1).r) / 255.0;
    float v3 = float(texelFetch(raw_data, base_idx + 2).r) / 255.0;

    float t = smoothstep(0.0, 1.0, morph_factor);

    // blend_p bestimmt den Grad der "Flachheit"
    // Wenn u_is_projected=1, animieren wir von 0 (3D) nach 1 (2D)
    float blend_p = (u_is_projected == 1) ? t : (1.0 - t);

    // Berechne Misch-Positionen für das alte und das neue System
    vec3 from_3d   = get_3d_pos(coord_system_from, v1, v2, v3);
    vec3 from_proj = get_proj_pos(coord_system_from, v1, v2, v3);
    vec3 pos_from  = mix(from_3d, from_proj, blend_p);

    vec3 to_3d     = get_3d_pos(coord_system_to, v1, v2, v3);
    vec3 to_proj   = get_proj_pos(coord_system_to, v1, v2, v3);
    vec3 pos_to    = mix(to_3d, to_proj, blend_p);

    // Finale Position (Übergang zwischen Systemen)
    gl_Position = mvp * vec4(mix(pos_from, pos_to, t), 1.0);
    gl_PointSize = u_point_size;
}
