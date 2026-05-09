# GEMINI.md - Binary Visualizer (C & GLFW)

## 1. Projekt-Vision
- **Ziel**: Ein Tool zur Visualisierung von Binärdaten (Reverse Engineering / Pattern Recognition), inspiriert von CantorDust.
- **Kern-Funktion**: Umwandlung von Dateibytes in 2D- (Digram) und 3D-Koordinaten (Trigram).
- **Architektur**: C (C11), GLFW für das Windowing, OpenGL 3.3+ (Core Profile), `cglm` für Mathematik.

## 2. Technische Spezifikationen
- **Sprache**: Reines C (kein C++), Fokus auf Performance und Speicher-Mapping.
- **Rendering**:
    - Primär: `GL_POINTS` für massive Datenmengen.
    - Sekundär: Instanced Cubes für Detailansichten.
- **Datenfluss**:
    1. Datei via `mmap` oder `fread` in Speicher laden.
    2. Bytes direkt in ein Vertex Buffer Object (VBO) schieben.
    3. Shader berechnet die räumliche Position ($x, y, z = B_i, B_{i+1}, B_{i+2}$).

## 3. Road-Map & Meilensteine
- [x] **Phase 1**: Basis-Window mit GLFW & OpenGL Kontext. Shader-Loader schreiben.
- [ ] **Phase 2**: Einlesen einer Datei und Darstellung als 2D-Punktwolke (X/Y).
- [ ] **Phase 3**: Integration von `cglm` für 3D-Kamera (Rotation/Zoom).
- [ ] **Phase 4**: Implementierung des 3D-Trigram-Modus (X/Y/Z).

## 4. Coding-Standards
- **Shaders**: Getrennte `.glsl` Dateien für Vertex- und Fragment-Shader.
- **Memory**: Jeder `malloc` bekommt ein entsprechendes `free`.
- **Naming**: `snake_case` für Funktionen und Variablen.

## 5. Bekannte Probleme & Notizen
- *Aktueller Stand*: Phase 1 abgeschlossen. Build-System steht, OpenGL-Kontext und Shader-Loader funktionieren.
- *Nächster Schritt*: Implementierung von `mmap` zum Einlesen von Dateien und Initialisierung des Punktwolken-Renderings (Phase 2).

## 6. Bemerkungen zum Workflow
- **Diskussions-Modus**: Wenn ein Satz mit einem **"D"** endet, dient dies als Signal für eine reine Diskussion. In diesem Fall sollen keine Code-Anpassungen oder Änderungen am Projekt vorgenommen werden.
