#ifndef SHADER_UTILS_H
#define SHADER_UTILS_H

#include <glad/glad.h>

char* read_shader_source(const char* path);
GLuint compile_shader(GLenum type, const char* source);
GLuint create_shader_program(const char* vertexPath, const char* fragmentPath);

#endif
