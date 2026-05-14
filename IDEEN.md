# IDEEN.md - Zukünftige Features & Verbesserungen

Hier werden Konzepte und Ideen festgehalten, die über den aktuellen Meilenstein hinausgehen.

## 1. Interaktive Entropie-Navigation
- **Konzept**: Der am unteren Rand dargestellte Entropie-Graph soll interaktiv werden.
- **Details**:
    - Anzeige von zwei verschiebbaren Markern (Dreiecke/Cursor) auf dem Graphen.
    - Mit der Maus kann ein Bereich ausgewählt werden.
    - Die Hauptansicht (Punktwolke) soll sich automatisch auf den gewählten Offset in der Datei synchronisieren.
    - Visualisierung von "Slices" der Datei basierend auf der Entropie-Auswahl.

## 2. Erweitertes Shading & Point-Effects
- **Idee**: Punkte basierend auf ihrer Entropie einfärben oder animieren.
- **Glitch-Detektor**: Automatisches Markieren von Bereichen mit ungewöhnlich niedriger oder hoher Entropie.

## 3. Datei-Vergleich (A/B Modus)
- **Konzept**: Zwei Dateien gleichzeitig laden und die Entropie-Differenzen oder Punktwolken-Abweichungen visualisieren.
- Hilfreich für das Diffing von Firmware-Versionen oder Protokoll-Dumps.

## 4. Export-Funktion
- Export der generierten Punktwolken als `.obj` oder `.ply` für die Analyse in externen Tools wie Blender oder MeshLab.
