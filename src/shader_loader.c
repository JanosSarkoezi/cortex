#include <stdio.h>
#include <stdlib.h>

char* read_shader_source(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Fehler: Shader-Datei %s konnte nicht geöffnet werden.\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(length + 1);
    if (buffer) {
        fread(buffer, 1, length, file);
        buffer[length] = '\0';
    }

    fclose(file);
    return buffer;
}
