#ifndef ENTROPY_H
#define ENTROPY_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    float* values;
    size_t num_blocks;
    size_t block_size;
} EntropyMap;

EntropyMap calculate_entropy_map(const uint8_t* data, size_t size, size_t block_size);
void free_entropy_map(EntropyMap em);

#endif
