#include "histogram.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Histogram calculate_histogram(const uint8_t* data, size_t size) {
    Histogram h = {NULL, 0, 0};
    if (size < 3) return h;

    // Wir nutzen ein flaches Array für 256x256x256 (16MB bei uint32_t)
    uint32_t* grid = (uint32_t*)calloc(256 * 256 * 256, sizeof(uint32_t));
    if (!grid) return h;

    printf("\033[1;34m[Cortex]\033[0m Histogramm wird berechnet...\n");

    for (size_t i = 0; i < size - 2; i++) {
        uint32_t idx = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        grid[idx]++;
    }

    // Zählen, wie viele Zellen belegt sind und was der Max-Wert ist
    size_t active_cells = 0;
    uint32_t max_val = 0;
    for (size_t i = 0; i < 256 * 256 * 256; i++) {
        if (grid[i] > 0) {
            active_cells++;
            if (grid[i] > max_val) max_val = grid[i];
        }
    }

    h.points = (HistogramPoint*)malloc(active_cells * sizeof(HistogramPoint));
    if (!h.points) {
        free(grid);
        return h;
    }

    size_t current = 0;
    for (uint32_t i = 0; i < 256 * 256 * 256; i++) {
        if (grid[i] > 0) {
            h.points[current].x = (uint8_t)((i >> 16) & 0xFF);
            h.points[current].y = (uint8_t)((i >> 8) & 0xFF);
            h.points[current].z = (uint8_t)(i & 0xFF);
            h.points[current].count = grid[i];
            current++;
        }
    }

    h.num_points = active_cells;
    h.max_count = max_val;

    free(grid);
    printf("\033[1;34m[Cortex]\033[0m Histogramm fertig: %zu eindeutige Punkte.\n", active_cells);
    return h;
}

void free_histogram(Histogram h) {
    if (h.points) free(h.points);
}
