#version 330 core

// 'aPos' erhält 3 Bytes (X, Y, Z) aus dem Buffer
layout (location = 0) in vec3 aPos;

// Die MVP-Matrix wird von unserem C-Code (via cglm) übergeben
uniform mat4 mvp;

void main() {
    // Falls wir die Daten NICHT bereits in C normalisiert haben,
    // skalieren wir sie hier von [0, 255] auf [-1.0, 1.0].
    // Hinweis: Wenn glVertexAttribPointer mit GL_TRUE genutzt wird,
    // erledigt OpenGL das Mapping auf [0, 1] automatisch für uns.
    vec3 position = aPos;

    // Die Standard-Transformation
    gl_Position = mvp * vec4(position, 1.0);

    // Punktgröße für die Sichtbarkeit
    gl_PointSize = 2.0;
}
