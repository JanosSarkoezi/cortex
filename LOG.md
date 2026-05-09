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

## Technische Details (Wissensbasis)
- **Shader**: `vertex.glsl` (Punkt-Generierung), `quad_fragment.glsl` (Post-Processing & Kontrast).
- **Datenstruktur**: `R8UI` für Rohdaten (TBO), `R32F` für Akkumulations-Textur (FBO), um hohe Zählwerte ohne Überlauf zu speichern.
