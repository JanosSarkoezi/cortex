#version 330 core
out vec4 FragColor;

void main() {
    // Ein klassisches "Hacker-Grün" oder Cyan für den Binary-Look
    // RGBA: (Rot, Grün, Blau, Alpha/Transparenz)
    FragColor = vec4(0.0, 1.0, 0.8, 1.0);
}
