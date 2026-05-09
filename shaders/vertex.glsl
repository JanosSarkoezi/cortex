#version 330 core

uniform usamplerBuffer raw_data;
uniform mat4 mvp;
uniform int mode_3d;

void main() {
    uint b1 = texelFetch(raw_data, gl_VertexID).r;
    uint b2 = texelFetch(raw_data, gl_VertexID + 1).r;
    
    float x = float(b1) / 255.0 * 2.0 - 1.0;
    float y = float(b2) / 255.0 * 2.0 - 1.0;
    float z = 0.0;

    if (mode_3d != 0) {
        uint b3 = texelFetch(raw_data, gl_VertexID + 2).r;
        z = float(b3) / 255.0 * 2.0 - 1.0;
    }

    gl_Position = mvp * vec4(x, y, z, 1.0);
    gl_PointSize = 1.0;
}
