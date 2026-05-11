#version 330 core
in float vCount;
out float FragValue;

void main() {
    // Bei Histogramm nutzen wir die normalisierte Häufigkeit
    // Bei Rohdaten bleibt es 1.0 (wird durch Blending summiert)
    FragValue = vCount;
}
