#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stddef.h>

typedef struct {
    unsigned char* data;
    size_t size;
    int fd;
} mapped_file;

mapped_file map_file(const char* path);
void unmap_file(mapped_file mf);

#endif
