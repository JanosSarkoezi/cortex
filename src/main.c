#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include "shader_utils.h"
#include "file_utils.h"
#include "camera.h"
#include <stdio.h>
#include <stdlib.h>

void setup_fbo(GLuint* fbo, GLuint* tex, int width, int height) {
    glGenFramebuffers(1, fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo);

    glGenTextures(1, tex);
    glBindTexture(GL_TEXTURE_2D, *tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Fehler: Framebuffer ist nicht vollständig!\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float exposure = 1.0f;
float offset = 0.5f;
int needs_update = 1;
Camera cam;

double last_x, last_y;
int left_mouse_pressed = 0;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)xoffset;
    int ctrl_pressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                       glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;

    if (ctrl_pressed) {
        if (yoffset > 0) offset *= 1.1f;
        else offset *= 0.9f;
        if (offset < 0.00001f) offset = 0.00001f;
        if (offset > 10.0f) offset = 10.0f;
        printf("Offset: %.6f\n", offset);
    } else {
        camera_zoom(&cam, (float)yoffset);
        needs_update = 1;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)mods;
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            left_mouse_pressed = 1;
            glfwGetCursorPos(window, &last_x, &last_y);
        } else {
            left_mouse_pressed = 0;
        }
    }
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
    if (left_mouse_pressed) {
        float dx = (float)(xpos - last_x);
        float dy = (float)(ypos - last_y);
        last_x = xpos;
        last_y = ypos;

        camera_rotate(&cam, dx, dy);
        needs_update = 1;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_Q) glfwSetWindowShouldClose(window, GLFW_TRUE);
        if (key == GLFW_KEY_SPACE) {
            cam.mode_3d = !cam.mode_3d;
            needs_update = 1;
            printf("Modus gewechselt: %s\n", cam.mode_3d ? "3D (Trigram)" : "2D (Digram)");
        }
        if (key == GLFW_KEY_R) {
            camera_reset(&cam);
            needs_update = 1;
            printf("Kamera zurückgesetzt.\n");
        }
    }
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
    
    camera_init(&cam);
    
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    GLuint accProgram = create_shader_program("shaders/vertex.glsl", "shaders/fragment.glsl");
    GLuint quadProgram = create_shader_program("shaders/quad_vertex.glsl", "shaders/quad_fragment.glsl");

    const char* target_file = (argc > 1) ? argv[1] : argv[0];
    mapped_file mf = map_file(target_file);
    if (mf.data == NULL) return -1;

    GLuint tbo_buffer, tbo_tex;
    glGenBuffers(1, &tbo_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo_buffer);
    glBufferData(GL_TEXTURE_BUFFER, mf.size, mf.data, GL_STATIC_DRAW);

    glGenTextures(1, &tbo_tex);
    glBindTexture(GL_TEXTURE_BUFFER, tbo_tex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R8UI, tbo_buffer);

    GLuint VAO;
    glGenVertexArrays(1, &VAO);

    float quadVertices[] = {
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

    GLuint fbo, accTex;
    setup_fbo(&fbo, &accTex, width, height);

    glEnable(GL_PROGRAM_POINT_SIZE);

    while (!glfwWindowShouldClose(window)) {
        if (needs_update) {
            int screenW, screenH;
            glfwGetFramebufferSize(window, &screenW, &screenH);
            cam.aspect = (float)screenW / (float)screenH;

            mat4 mvp;
            camera_get_mvp(&cam, mvp);

            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glViewport(0, 0, width, height);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);

            glUseProgram(accProgram);
            glUniformMatrix4fv(glGetUniformLocation(accProgram, "mvp"), 1, GL_FALSE, (float*)mvp);
            glUniform1i(glGetUniformLocation(accProgram, "mode_3d"), cam.mode_3d);
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_BUFFER, tbo_tex);
            glUniform1i(glGetUniformLocation(accProgram, "raw_data"), 0);

            glBindVertexArray(VAO);
            GLsizei count = (cam.mode_3d) ? (GLsizei)(mf.size - 2) : (GLsizei)(mf.size - 1);
            if (count > 0) {
                glDrawArrays(GL_POINTS, 0, count);
            }

            glDisable(GL_BLEND);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            needs_update = 0;
        }

        int screenW, screenH;
        glfwGetFramebufferSize(window, &screenW, &screenH);
        glViewport(0, 0, screenW, screenH);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(quadProgram);
        glUniform1f(glGetUniformLocation(quadProgram, "exposure"), exposure);
        glUniform1f(glGetUniformLocation(quadProgram, "offset"), offset);
        
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, accTex);
        glUniform1i(glGetUniformLocation(quadProgram, "screenTexture"), 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    unmap_file(mf);
    glfwTerminate();
    return 0;
}
