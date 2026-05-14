#version 330 core
layout(location = 0) in vec2 aPos;
void main() {
    // aPos.x ist -0.9 bis 0.9
    // aPos.y ist 0.0 bis 1.0 (Entropie)
    // Wir zeichnen am unteren Rand
    float y = -0.85 + (aPos.y * 0.15); 
    gl_Position = vec4(aPos.x, y, 0.0, 1.0);
}
