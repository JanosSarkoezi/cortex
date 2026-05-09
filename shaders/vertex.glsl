#version 330 core

// Das Texture Buffer Object mit den Rohdaten
uniform usamplerBuffer raw_data;
uniform mat4 mvp;

void main() {
    // Holen der Bytes für das Sliding Window (Fenstergröße 2 für 2D)
    // texelFetch holt den Wert am Index gl_VertexID
    uint b1 = texelFetch(raw_data, gl_VertexID).r;
    uint b2 = texelFetch(raw_data, gl_VertexID + 1).r;

    // Normalisierung auf [0, 1] (Bytes sind 0-255)
    float x = float(b1) / 255.0;
    float y = float(b2) / 255.0;

    // Mapping auf [-1, 1] für den OpenGL-Clip-Space
    vec3 position = vec3(x, y, 0.0) * 2.0 - 1.0;

    gl_Position = mvp * vec4(position, 1.0);
    gl_PointSize = 1.0; 
}
