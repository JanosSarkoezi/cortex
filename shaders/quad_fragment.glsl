#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float exposure; // Regler für die Helligkeit

void main() {
    float count = texture(screenTexture, TexCoords).r;
    
    // Logarithmische Skalierung
    // Helligkeit = log(1 + Treffer * Intensität) / log(Anpassung)
    float brightness = log(1.0 + count * exposure) / log(100.0);
    
    // Wir mappen das auf ein schönes Grün-Spektrum
    vec3 lowColor = vec3(0.0, 0.2, 0.1);    // Sehr dunkles Petrol/Grün
    vec3 highColor = vec3(0.0, 1.0, 0.4);   // Helles Matrix-Grün
    
    vec3 finalColor = mix(vec3(0.0), mix(lowColor, highColor, brightness), step(0.0001, brightness));
    
    FragColor = vec4(finalColor, 1.0);
}
