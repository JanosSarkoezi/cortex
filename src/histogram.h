#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t x, y, z;
    uint32_t count;
} HistogramPoint;

typedef struct {
    HistogramPoint* points;
    size_t num_points;
    uint32_t max_count;
} Histogram;

Histogram calculate_histogram(const uint8_t* data, size_t size);
void free_histogram(Histogram h);

#endif
