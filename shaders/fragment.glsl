#version 330 core
out float FragValue;

void main() {
    // Wir geben einfach 1.0 aus. 
    // Durch additives Blending summiert sich das im Framebuffer auf.
    FragValue = 1.0;
}
