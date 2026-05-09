#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>

// Hilfsfunktion zum Kompilieren und Prüfen eines einzelnen Shaders
GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Fehlerprüfung
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::COMPILATION_FAILED\n%s\n", infoLog);
    }
    return shader;
}

// Die Hauptfunktion zum Erstellen des Programms
GLuint create_shader_program(const char* vertexPath, const char* fragmentPath) {
    // 1. Quellcode einlesen (mit der Funktion aus unserem vorletzten Schritt)
    char* vertexCode = read_shader_source(vertexPath);
    char* fragmentCode = read_shader_source(fragmentPath);

    if (!vertexCode || !fragmentCode) return 0;

    // 2. Shader einzeln kompilieren
    GLuint vertexShader = compile_shader(GL_VERTEX_SHADER, vertexCode);
    GLuint fragmentShader = compile_shader(GL_FRAGMENT_SHADER, fragmentCode);

    // 3. Program erstellen und Linken
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Link-Fehler prüfen
    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }

    // 4. Aufräumen: Die einzelnen Shader-Objekte werden nicht mehr gebraucht
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    free(vertexCode);
    free(fragmentCode);

    return shaderProgram;
}
