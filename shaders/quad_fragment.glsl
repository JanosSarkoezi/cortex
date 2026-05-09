#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float exposure; // Globaler Multiplikator
uniform float offset;   // Kontrast / Threshold-Filter

void main() {
    float count = texture(screenTexture, TexCoords).r;

    // Neue Transformations-Formel: -log(1/count + offset)
    // Wir nutzen max(count, 0.0001) um Division durch 0 zu vermeiden
    float brightness = -log(1.0 / max(count, 0.0001) + offset) * exposure;

    brightness = clamp(brightness, 0.0, 1.0);

    // Matrix-Style Grün-Spektrum
    vec3 lowColor = vec3(0.0, 0.2, 0.05);
    vec3 highColor = vec3(0.2, 1.0, 0.5);

    vec3 finalColor = mix(vec3(0.0), mix(lowColor, highColor, brightness), step(0.01, brightness));

    FragColor = vec4(finalColor, 1.0);
}
