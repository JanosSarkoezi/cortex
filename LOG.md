# LOG.md - Cortex Binary Visualizer

## Projekt-Fortschritt & Entscheidungen

### 2026-05-09
- **Initialisierung**: Phase 1 abgeschlossen. Build-System (Meson), Shader-Loader und OpenGL-Kontext stehen.
- **Exit-Shortcut**: Taste `q` zum Beenden des Programms hinzugefügt (via `key_callback`).
- **Workflow-Regel**: "D"-Modus in `GEMINI.md` definiert (reine Diskussion am Satzende mit "D").
- **Intensitäts-Mapping**: Neue Formel `-log(1/count + offset) * exposure` implementiert, um Strukturen in dichten Daten besser hervorzuheben.
- **Interaktive Steuerung**: 
    - Mausrad: Exposure (Helligkeit).
    - STRG + Mausrad: Offset (Kontrast/Rauschfilter).
- **Sliding Window (GPU)**: Umstellung von statischen VBOs auf Texture Buffer Objects (TBO). Der Vertex-Shader nutzt `gl_VertexID` und `texelFetch`, um überlappende Byte-Paare (B_i, B_{i+1}) live zu lesen. Maximale Datenabdeckung erreicht.
- **Hybrid-Rendering (Performance-Fix)**: 
    - Problem: 100M+ Punkte pro Frame machten die Maus träge.
    - Lösung: Der schwere Akkumulations-Pass (Heatmap-Generierung) läuft nun statisch (einmalig). Der Main-Loop zeigt nur das fertige Textur-Resultat an.
    - Ergebnis: Butterweiche Mausbewegung bei voller Detailtiefe.
- **Kamera-Refactoring**:
    - Umstellung von Euler-Winkeln (Yaw/Pitch) auf **Quaternions** (`versor`).
    - Behebung des Gimbal-Lock-Problems.
    - Implementierung einer hybriden Rotation: Lokaler Pitch und globaler Yaw für intuitive Bedienung.
    - Nutzung von `cglm` nativen Quaternion-Funktionen für Stabilität und Performance.

### 2026-05-10
- **Colormaps**: Integration von Turbo und Viridis via mathematischer GLSL-Approximation.
- **Tab-Wechsel**: Taste `TAB` schaltet zwischen Matrix-Grün, Turbo und Viridis um.
- **UI-Ebene**: Neue Layer für HUD-Elemente hinzugefügt.
    - Taste `H`: Blendet die UI ein/aus.
    - Colormap-Skala: Ein vertikaler Balken am rechten Rand zeigt die aktuelle Farbskala (Turbo, Viridis oder Matrix-Grün).
- **Shader-Update**: `ui_vertex.glsl` und `ui_fragment.glsl` für prozedurale UI-Elemente implementiert.
- **Koordinatensysteme**: Kartesisch, Zylindrisch und Sphärisch hinzugefügt.
    - Taste `K`: Wechselt das Mapping der Bytes auf die Raumkoordinaten.
    - **Morph-Animation**: Fließende Übergänge zwischen den Systemen via linearer Interpolation auf der GPU.
    - `smoothstep`: Nutzt eine sanfte Beschleunigung/Verzögerung für die Morph-Animation.
    - Dauer: 0.6 Sekunden, gesteuert durch die CPU via `glfwGetTime()`.
- **Punktgröße**: Einstellbar zwischen 1.0 und 4.0.
    - Tasten `+` / `-` (und Numpad): Ändert die Größe der dargestellten Datenpunkte.
    - `vertex.glsl`: Nutzt `u_point_size` zur Steuerung von `gl_PointSize`.
- **Shader-Embedding**: Alle GLSL-Shader wurden direkt in den C-Binary eingebettet.
    - `src/shaders_embedded.h`: Enthält alle Shader-Sourcen als statische Strings.
    - Portabilität: Das Programm benötigt das Verzeichnis `shaders/` zur Laufzeit nicht mehr.
    - Refactoring: `create_shader_program_from_source` wurde hinzugefügt, um Shader direkt aus dem Speicher zu laden.
- **Projektionsebenen**:
    - Tasten `1`, `2`, `3`: Sofortiges Ausrichten der Kamera auf XY-, YZ- oder ZX-Ebene.
    - Funktioniert sowohl im 2D (Ortho) als auch im 3D (Perspektive) Modus.
    - `vertex.glsl`: Berechnet nun immer Trigramm-Koordinaten, um Projektions-Wechsel in allen Modi zu unterstützen.
- **Rotation-Feinschliff**:
    - Optimierung der Kamera-Rotation auf eine reine **Arcball/Welt-Raum Logik**.
    - Durch Linksmultiplikation der Delta-Quaternions bleibt die Rotation immer viewport-relativ.
    - Ergebnis: Intuitivere Untersuchung von abstrakten Datenstrukturen ohne festen "Up-Vector".
- **Kamera-Animation**:
    - `glm_quat_slerp` für flüssige Übergänge beim Wechsel der Projektionsebenen und beim Reset (`R`).
    - Präzisions-Fix: Explizites Setzen der Ziel-Orientierung nach Abschluss der Animation zur Vermeidung von Rundungsfehlern.

### 2026-05-11
- **Mathematische Korrektur der Punktverteilung**:
    - **Sphärisches System**:
        - Vertikale Verteilung ($\phi$): Umstellung von linearer Abbildung (`v2 * PI`) auf `acos(1.0 - 2.0 * v2)`. Verhindert das "Clustering" an den Polen (Orange-Peel-Effekt).
        - Radiale Verteilung ($r$): Anwendung der Kubikwurzel `pow(v3, 1/3)`. Sorgt für konstante Punktdichte im gesamten 3D-Volumen der Kugel.
    - **Zylindrisches System**:
        - Radiale Verteilung ($r$): Anwendung der Quadratwurzel `sqrt(v2)`. Gewährleistet eine homogene Verteilung über die Kreisfläche (Grundfläche des Zylinders).
    - **Ziel**: Elimination von Artefakten, die durch die Koordinatentransformation entstehen. Reine Zufallsdaten erscheinen nun als homogene Wolken, wodurch echte Muster in Binärdaten (Strukturen/Offsets) deutlicher hervortreten.
- **Entkopplung der Morph-Animation**:
    - Problem: Systemwechsel verursachten ein ungewolltes "Aufblähen/Kollabieren", da die Projektions-Animation fälschlicherweise mitgetriggert wurde.
    - Lösung: Einführung von getrennten Zuständen für Start- und Zielprojektion (`u_proj_from`, `u_proj_to`) im Shader. Systemwechsel bewahren nun den aktuellen Projektionsgrad.
    - Animation: `morph_duration` auf 2.0s erhöht für flüssigere, besser analysierbare Übergänge.
- **3D-Histogramm Modus (Voxel-Rendering)**:
    - Konzept: Aggregation von Byte-Tripletts zu einer Häufigkeitsmatrix ($256^3$).
    - Performance: Reduziert die zu zeichnenden Punkte bei großen Dateien massiv (maximal 16,7 Mio. Punkte).
    - Implementierung: CPU-seitige Vorberechnung des Histogramms; Upload als VBO; Shader-Update zur Nutzung von Häufigkeitswerten für die Intensität.
    - Automatisierung: Automatischer Wechsel in den Histogramm-Modus bei Dateien > 20MB. Manuelle Umschaltung via Taste `m`.

### 2026-05-12
- **Text HUD (stb_truetype)**:
    - **Integration**: `stb_truetype.h` zur Generierung von Font-Bitmaps integriert.
    - **JetBrains Mono**: Hochwertiger Monospace-Font für optimale Lesbarkeit im technischen Kontext eingebettet.
    - **Binary Embedding**: Neues Skript `scripts/embed_binary.py` erstellt, das TTF-Dateien in C-Header-Dateien (Hex-Arrays) umwandelt. Die Binary bleibt somit portabel und benötigt keine externen Font-Dateien.
    - **Text-Renderer**: Neues Modul `src/text_renderer.c` implementiert. Nutzt Batch-Rendering (Sammeln aller Buchstaben-Quads in einem VBO) für maximale Performance.
    - **Dynamisches HUD**: Echtzeit-Anzeige von Koordinatensystem, Kamera-Modus und Projektionsstatus.
    - **Shader**: Eigener Text-Shader (`text_vertex.glsl`, `text_fragment.glsl`) für Rendering mit Alpha-Masken aus GL_RED Texturen.

## Technische Details (Wissensbasis)
- **Shader**: `vertex.glsl` (Punkt-Generierung), `quad_fragment.glsl` (Post-Processing & Kontrast).
- **Datenstruktur**: `R8UI` für Rohdaten (TBO), `R32F` für Akkumulations-Textur (FBO), um hohe Zählwerte ohne Überlauf zu speichern.
