#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 mvp;

void main() {
    // Da wir im C-Code 'normalized = GL_TRUE' nutzen, kommt aPos als [0.0, 1.0] an.
    // Wir mappen das auf [-1.0, 1.0], damit es zentriert ist.
    vec3 position = aPos * 2.0 - 1.0;

    gl_Position = mvp * vec4(position, 1.0);
    gl_PointSize = 1.0;
}
