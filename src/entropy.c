#include "entropy.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

EntropyMap calculate_entropy_map(const uint8_t* data, size_t size, size_t block_size) {
    EntropyMap em = {NULL, 0, block_size};
    if (size < block_size) return em;

    em.num_blocks = size / block_size;
    em.values = (float*)malloc(em.num_blocks * sizeof(float));
    if (!em.values) return em;

    for (size_t b = 0; b < em.num_blocks; b++) {
        uint32_t counts[256] = {0};
        const uint8_t* block_data = data + (b * block_size);
        
        for (size_t i = 0; i < block_size; i++) {
            counts[block_data[i]]++;
        }

        float entropy = 0.0f;
        for (int i = 0; i < 256; i++) {
            if (counts[i] > 0) {
                float p = (float)counts[i] / (float)block_size;
                entropy -= p * log2f(p);
            }
        }
        // Normalisieren auf 0.0 - 1.0 (8.0 ist max Entropie bei Byte-Daten)
        em.values[b] = entropy / 8.0f;
    }

    return em;
}

void free_entropy_map(EntropyMap em) {
    if (em.values) free(em.values);
}
