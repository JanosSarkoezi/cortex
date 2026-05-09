#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include "shader_utils.h"
#include "file_utils.h"
#include <stdio.h>
#include <stdlib.h>

void setup_fbo(GLuint* fbo, GLuint* tex, int width, int height) {
    glGenFramebuffers(1, fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo);

    glGenTextures(1, tex);
    glBindTexture(GL_TEXTURE_2D, *tex);
    // Wir nutzen GL_R32F für hohe Präzision beim Zählen (32-Bit Float, nur Rot-Kanal)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Fehler: Framebuffer ist nicht vollständig!\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float exposure = 50.0f; // Höherer Startwert

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;
    (void)xoffset;
    if (yoffset > 0) exposure *= 1.3f; // Größerer Schritt
    else exposure *= 0.7f;

    if (exposure < 0.001f) exposure = 0.001f;
    if (exposure > 100000.0f) exposure = 100000.0f;

    printf("Exposure: %.2f\n", exposure);
}

int main(int argc, char** argv) {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int width = 1024, height = 1024;
    GLFWwindow* window = glfwCreateWindow(width, height, "Cortex - Binary Visualizer", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    // Shader laden
    GLuint accProgram = create_shader_program("shaders/vertex.glsl", "shaders/fragment.glsl");
    GLuint quadProgram = create_shader_program("shaders/quad_vertex.glsl", "shaders/quad_fragment.glsl");

    // Datei laden
    const char* target_file = (argc > 1) ? argv[1] : argv[0];
    mapped_file mf = map_file(target_file);
    if (mf.data == NULL) return -1;

    // Punkt-Daten Setup
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mf.size, mf.data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_TRUE, 3 * sizeof(unsigned char), (void*)0);
    glEnableVertexAttribArray(0);

    // Fullscreen Quad Setup
    float quadVertices[] = {
        // Positions   // TexCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // Framebuffer Setup
    GLuint fbo, accTex;
    setup_fbo(&fbo, &accTex, width, height);

    glEnable(GL_PROGRAM_POINT_SIZE);
    mat4 mvp = GLM_MAT4_IDENTITY_INIT;

    while (!glfwWindowShouldClose(window)) {
        // 1. Pass: Akkumulation
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, width, height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE); // Additives Blending: Werte summieren sich

        glUseProgram(accProgram);
        glUniformMatrix4fv(glGetUniformLocation(accProgram, "mvp"), 1, GL_FALSE, (float*)mvp);
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, (mf.size / 3));

        glDisable(GL_BLEND);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. Pass: Visualisierung (Post-Processing)
        int screenW, screenH;
        glfwGetFramebufferSize(window, &screenW, &screenH);
        glViewport(0, 0, screenW, screenH);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(quadProgram);
        glUniform1f(glGetUniformLocation(quadProgram, "exposure"), exposure); // Basis-Intensität
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, accTex);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup...
    unmap_file(mf);
    glfwTerminate();
    return 0;
}
