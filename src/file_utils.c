#include "file_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

mapped_file map_file(const char* path) {
    mapped_file mf = {NULL, 0, -1};

    mf.fd = open(path, O_RDONLY);
    if (mf.fd == -1) {
        perror("Fehler beim Öffnen der Datei");
        return mf;
    }

    struct stat st;
    if (fstat(mf.fd, &st) == -1) {
        perror("Fehler beim Abrufen der Dateigröße");
        close(mf.fd);
        mf.fd = -1;
        return mf;
    }

    mf.size = st.st_size;
    mf.data = mmap(NULL, mf.size, PROT_READ, MAP_PRIVATE, mf.fd, 0);
    if (mf.data == MAP_FAILED) {
        perror("Fehler beim mmap");
        close(mf.fd);
        mf.fd = -1;
        mf.data = NULL;
        return mf;
    }

    return mf;
}

void unmap_file(mapped_file mf) {
    if (mf.data != NULL) {
        munmap(mf.data, mf.size);
    }
    if (mf.fd != -1) {
        close(mf.fd);
    }
}
