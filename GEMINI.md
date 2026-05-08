# GEMINI.md - Binary Visualizer (C & GLFW)

## 1. Projekt-Vision
- **Ziel**: Ein Tool zur Visualisierung von Binärdaten (Reverse Engineering / Pattern Recognition), inspiriert von CantorDust.
- **Kern-Funktion**: Umwandlung von Dateibytes in 2D- (Digram) und 3D-Koordinaten (Trigram).
- **Architektur**: C (C11), GLFW für das Windowing, OpenGL 3.3+ (Core Profile), `cglm` für Mathematik.

## 2. Technische Spezifikationen & Build-System
- **Build-System**: Meson (mit `meson.build`).
- **Sprache**: Reines C (C11 Standard).
- **Abhängigkeiten**:
    - `glfw3`: Fensterverwaltung & Input.
    - `glad` oder `glew`: OpenGL Function Loader.
    - `cglm`: Mathematik-Bibliothek für 3D-Operationen.
- **Ordnerstruktur**:
    - `src/`: Quellcode (`.c`, `.h`).
    - `shaders/`: GLSL Code.
    - `subprojects/`: Externe Libraries (Meson Wraps).

## 3. Road-Map & Meilensteine
- [ ] **Phase 1**: Meson-Projekt aufsetzen, `glfw` einbinden und ein schwarzes Fenster öffnen.
- [ ] **Phase 2**: Shader-Lade-Logik implementieren (Vertex/Fragment).
- [ ] **Phase 3**: Datei-Einlesen (mmap) und 2D-Punktwolke rendern.
- [ ] **Phase 4**: 3D-Integration mit `cglm` (Kamera-Steuerung).

## 4. Coding-Standards
- **Naming**: `snake_case` für Funktionen und Variablen.
- **Error Handling**: Überprüfung von Datei-Handles und Shader-Kompilierung.
- **Build-Workflow**: `meson setup build` -> `meson compile -C build`.

## 5. Dokumentation & Fortschritt
- *Aktueller Stand*: Build-System auf Meson festgelegt, Library-Management via `subprojects/`.
- *Nächster Schritt*: Erstellung der `meson.build` Datei.
