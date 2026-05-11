# Cortex - Binary Visualizer

Cortex ist ein Hochleistungs-Tool zur Visualisierung von Binärdaten, das darauf ausgelegt ist, die menschliche Fähigkeit zur Mustererkennung zu nutzen. Durch die Abbildung von Dateibytes auf räumliche Koordinaten (2D-Digramme und 3D-Trigramme) werden verborgene Strukturen in Daten für das menschliche Auge sichtbar.

Der Name **Cortex** leitet sich von der Idee ab, dass das Programm lediglich die Daten aufbereitet, während das Gehirn des Benutzers (der visuelle Cortex) als eigentliche "Pattern Recognition Engine" fungiert.

## Technische Highlights

- **Sprache**: Rein in C (C11) geschrieben.
- **Rendering**: OpenGL 3.3 (Core Profile) mit Fokus auf massive Punktwolken.
- **Performance**: Nutzt Texture Buffer Objects (TBO) für schnelles Daten-Streaming und ein Akkumulations-Rendering-Verfahren für flüssige Interaktion bei Millionen von Punkten.
- **Mathematik**: Präzise Koordinatentransformationen (Kartesisch, Zylindrisch, Sphärisch) mit Korrekturen für gleichmäßige Punktverteilung.
- **Animation**: GPU-beschleunigte Morphing-Effekte zwischen den Systemen.

## Voraussetzungen

- C-Compiler (gcc oder clang)
- Meson & Ninja (Build-System)
- GLFW 3
- OpenGL 3.3+ Support

## Installation & Start

1. Repository klonen.
2. Build-Verzeichnis erstellen: `meson setup build`
3. Kompilieren: `meson compile -C build`
4. Ausführen: `./build/cortex <dateipfad>`

## Steuerung

### Programm
- **q**: Programm beenden
- **m**: Modus umschalten (Sequenziell vs. 3D-Histogramm)
- **h**: UI (HUD) und Farbskala ein-/ausblenden

### Ansicht & Projektion
- **space**: Wechsel zwischen perspektivischer (3D) und orthografischer (2D) Kamera.
- **p**: Projektion umschalten (3D-Punktwolke zu flacher Projektion).
- **1 / 2 / 3**: Schnelle Ausrichtung der Kamera auf XY-, YZ- oder ZX-Ebene.
- **r**: Kamera-Rotation und Zoom zurücksetzen.

### Koordinatensysteme (Morphing)
- **k**: Kartesisches System (X, Y, Z)
- **z**: Zylindrisches System (Radius, Winkel, Höhe)
- **s**: Sphärisches System (Radius, Azimut, Polarwinkel)

### Darstellung
- **tab**: Colormap wechseln (Matrix-Grün, Turbo, Viridis).
- **+ / -**: Punktgröße anpassen.
- **Mausrad**: Zoom.
- **STRG + Mausrad**: Kontrast / Rauschfilter (Offset) anpassen.
- **Linksklick + Ziehen**: Kamera rotieren.

## Philosophie

Binärdaten wirken oft chaotisch. Cortex ordnet sie in geometrischen Räumen an. Ein verschlüsselter Datenstrom erscheint als homogenes Rauschen, während unkomprimierte Bilder, ausführbare Dateien oder strukturierte Protokolle sofort erkennbare geometrische Signaturen hinterlassen. Cortex ist das Werkzeug, um diese Signaturen zu finden.
