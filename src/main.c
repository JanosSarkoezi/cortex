#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include "shader_utils.h"
#include "shaders_embedded.h"
#include "file_utils.h"
#include "camera.h"
#include "histogram.h"
#include "text_renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

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
float point_size = 1.0f;
int colormap_idx = 0; // 0: Matrix, 1: Turbo, 2: Viridis
int use_histogram = 0; // 0: Rohdaten, 1: Histogramm
Histogram hist = {NULL, 0, 0};
GLuint hist_vbo = 0, hist_vao = 0;
int coord_system_from = 0;
int coord_system_to = 0;
int proj_from = 0;
int proj_to = 0;
float morph_factor = 1.0f;
float morph_duration = 2.0f;
double last_morph_time = 0.0;
int show_ui = 1;
int needs_update = 1;
int current_view = 0;
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

void trigger_transition() {
    morph_factor = 0.0f;
    last_morph_time = glfwGetTime();
    needs_update = 1;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;

    // Wir reagieren nur auf den ersten Tastendruck
    if (action != GLFW_PRESS) return;

    // --- PROGRAMM-STEUERUNG ---
    if (key == GLFW_KEY_Q) glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_H) {
        show_ui = !show_ui;
        printf("\033[1;34m[UI]\033[0m %s\n", show_ui ? "eingeblendet" : "ausgeblendet");
    }

    // --- MODUS-WECHSEL (2D / 3D) ---
    if (key == GLFW_KEY_SPACE) {
        cam.mode_3d = !cam.mode_3d;
        needs_update = 1; // Nur Kamera-Update, kein Morphing nötig
        printf("\033[1;34m[Kamera]\033[0m %s\n", cam.mode_3d ? "Perspektivisch" : "Orthografisch");
    }

    if (key == GLFW_KEY_P) {
        proj_from = proj_to;
        proj_to = !proj_to;
        trigger_transition(); // Startet Morphing der Punkte
        printf("\033[1;34m[Projektion]\033[0m %s\n", proj_to ? "AN" : "AUS");
    }

    if (key == GLFW_KEY_1 || key == GLFW_KEY_2 || key == GLFW_KEY_3) {
        current_view = (key == GLFW_KEY_1) ? 0 : (key == GLFW_KEY_2 ? 1 : 2);
        camera_set_view(&cam, current_view); // Kamera ausrichten

        if (proj_to) {
            proj_from = proj_to; // Bleibe in Projektion
            trigger_transition(); // Nur morphen, wenn Projektion aktiv ist
        } else {
            needs_update = 1;
        }
    }

    // --- KOORDINATENSYSTEME ---
    if (key == GLFW_KEY_K || key == GLFW_KEY_Z || key == GLFW_KEY_S) {
        int target_sys = (key == GLFW_KEY_K) ? 0 : (key == GLFW_KEY_Z ? 1 : 2);

        if (coord_system_to != target_sys) {
            coord_system_from = coord_system_to;
            coord_system_to = target_sys;
            proj_from = proj_to; // Aktuellen Projektionszustand beibehalten
            trigger_transition(); // Morph zwischen den Systemen

            const char* names[] = {"Kartesisch", "Zylindrisch", "Sphärisch"};
            printf("\033[1;34m[System]\033[0m %s -> %s\n", names[coord_system_from], names[coord_system_to]);
        }
    }


    // --- KAMERA & DARSTELLUNG ---
    if (key == GLFW_KEY_R) {
        camera_reset(&cam); // Setzt Orientierung und Zoom zurück
        trigger_transition();
        printf("\033[1;34m[Reset]\033[0m Kamera zurückgesetzt.\n");
    }

    if (key == GLFW_KEY_TAB) {
        colormap_idx = (colormap_idx + 1) % 3;
        const char* names[] = {"Matrix", "Turbo", "Viridis"};
        printf("\033[1;34m[Farbe]\033[0m Colormap: %s\n", names[colormap_idx]);
        needs_update = 1;
    }

    if (key == GLFW_KEY_M) {
        if (hist.points != NULL) {
            use_histogram = !use_histogram;
            needs_update = 1;
            printf("\033[1;34m[Modus]\033[0m %s\n", use_histogram ? "3D-Histogramm (Voxel)" : "Sequenziell (Rohdaten)");
        } else {
            printf("\033[1;31m[Fehler]\033[0m Histogramm nicht verfügbar.\n");
        }
    }

    if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) {
        point_size = (point_size < 4.0f) ? point_size + 0.5f : 4.0f;
        needs_update = 1;
    }
    if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) {
        point_size = (point_size > 1.0f) ? point_size - 0.5f : 1.0f;
        needs_update = 1;
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

    GLuint accProgram = create_shader_program_from_source(vertex_shader_source, fragment_shader_source);
    GLuint quadProgram = create_shader_program_from_source(quad_vertex_shader_source, quad_fragment_shader_source);
    GLuint uiProgram = create_shader_program_from_source(ui_vertex_shader_source, ui_fragment_shader_source);

    if (argc < 2) {
        fprintf(stderr, "\033[1;36mCortex Binary Visualizer\033[0m\n");
        fprintf(stderr, "Nutzung: %s <dateipfad>\n\n", argv[0]);
        fprintf(stderr, "\033[1;33mSteuerung:\033[0m\n");
        fprintf(stderr, "  \033[1;32mspace\033[0m       - Perspektive umschalten (3D / Ortho)\n");
        fprintf(stderr, "  \033[1;32mp\033[0m           - Projektion umschalten (3D-Wolke / Flach)\n");
        fprintf(stderr, "  \033[1;32mtab\033[0m         - Colormap wechseln (Matrix, Turbo, Viridis)\n");
        fprintf(stderr, "  \033[1;32mk / z / s\033[0m   - System wechseln (Kartesisch, Zylindrisch, Sphärisch)\n");
        fprintf(stderr, "  \033[1;32m1 / 2 / 3\033[0m   - Ansicht ausrichten (XY, YZ, ZX)\n");
        fprintf(stderr, "  \033[1;32mh\033[0m           - UI / Farbskala ein-/ausblenden\n");
        fprintf(stderr, "  \033[1;32mr\033[0m           - Kamera & Zoom zurücksetzen\n");
        fprintf(stderr, "  \033[1;32m+ / -\033[0m       - Punktgröße anpassen (1.0 - 4.0)\n");
        fprintf(stderr, "  \033[1;32mq\033[0m           - Programm beenden\n\n");
        fprintf(stderr, "\033[1;33mMaus:\033[0m\n");
        fprintf(stderr, "  Scrollen    - Zoom\n");
        fprintf(stderr, "  STRG+Scroll - Kontrast / Rauschfilter (Offset)\n");
        fprintf(stderr, "  Links-Klick - Kamera rotieren\n\n");
        glfwTerminate();
        return 1;
    }

    const char* target_file = argv[1];
    mapped_file mf = map_file(target_file);
    if (mf.data == NULL) {
        glfwTerminate();
        return -1;
    }

    // Histogramm berechnen (Cortex-Mode)
    hist = calculate_histogram(mf.data, mf.size);
    if (hist.points) {
        glGenVertexArrays(1, &hist_vao);
        glGenBuffers(1, &hist_vbo);
        glBindVertexArray(hist_vao);
        glBindBuffer(GL_ARRAY_BUFFER, hist_vbo);
        glBufferData(GL_ARRAY_BUFFER, hist.num_points * sizeof(HistogramPoint), hist.points, GL_STATIC_DRAW);

        // Attribute 0: vec3 aPos (x, y, z als uint8_t, im Shader / 255.0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(HistogramPoint), (void*)0);
        // Attribute 1: float aCount (Häufigkeit)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(HistogramPoint), (void*)offsetof(HistogramPoint, count));

        // Automatisch in Histogramm-Modus wechseln bei großen Dateien (> 20MB)
        if (mf.size > 20 * 1024 * 1024) {
            use_histogram = 1;
            printf("\033[1;34m[Cortex]\033[0m Große Datei erkannt, Histogramm-Modus aktiviert.\n");
        }
    }

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

    // UI VAO für die Skala (Rechter Rand)
    float uiVertices[] = {
        0.85f,  0.8f,  0.0f, 1.0f,
        0.85f, -0.8f,  0.0f, 0.0f,
        0.95f, -0.8f,  1.0f, 0.0f,

        0.85f,  0.8f,  0.0f, 1.0f,
        0.95f, -0.8f,  1.0f, 0.0f,
        0.95f,  0.8f,  1.0f, 1.0f
    };
    GLuint uiVAO, uiVBO;
    glGenVertexArrays(1, &uiVAO);
    glGenBuffers(1, &uiVBO);
    glBindVertexArray(uiVAO);
    glBindBuffer(GL_ARRAY_BUFFER, uiVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uiVertices), &uiVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    GLuint fbo, accTex;
    setup_fbo(&fbo, &accTex, width, height);

    glEnable(GL_PROGRAM_POINT_SIZE);

    text_renderer_init();

    while (!glfwWindowShouldClose(window)) {
        double current_time = glfwGetTime();
        float dt = (float)(current_time - last_morph_time);
        last_morph_time = current_time;

        if (morph_factor < 1.0f) {
            morph_factor += dt / morph_duration;
            if (morph_factor >= 1.0f) {
                morph_factor = 1.0f;
                coord_system_from = coord_system_to;
            }
            needs_update = 1;
        }

        if (cam.anim_factor < 1.0f) {
            cam.anim_factor += dt / 0.5f;
            if (cam.anim_factor >= 1.0f) {
                cam.anim_factor = 1.0f;
                glm_quat_copy(cam.target_orientation, cam.orientation);
            }
            needs_update = 1;
        }

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

            glUniform1i(glGetUniformLocation(accProgram, "coord_system_from"), coord_system_from);
            glUniform1i(glGetUniformLocation(accProgram, "coord_system_to"), coord_system_to);
            glUniform1f(glGetUniformLocation(accProgram, "morph_factor"), morph_factor);
            glUniform1i(glGetUniformLocation(accProgram, "mode_3d"), cam.mode_3d);
            glUniform1i(glGetUniformLocation(accProgram, "projection_view"), current_view);
            glUniform1f(glGetUniformLocation(accProgram, "u_point_size"), point_size);
            glUniform1i(glGetUniformLocation(accProgram, "u_proj_from"), proj_from);
            glUniform1i(glGetUniformLocation(accProgram, "u_proj_to"), proj_to);
            glUniform1i(glGetUniformLocation(accProgram, "u_use_histogram"), use_histogram);

            if (use_histogram) {
                glBindVertexArray(hist_vao);
                glDrawArrays(GL_POINTS, 0, (GLsizei)hist.num_points);
            } else {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_BUFFER, tbo_tex);
                glUniform1i(glGetUniformLocation(accProgram, "raw_data"), 0);

                glBindVertexArray(VAO);
                GLsizei count = (cam.mode_3d) ? (GLsizei)(mf.size - 2) : (GLsizei)(mf.size - 1);
                if (count > 0) {
                    glDrawArrays(GL_POINTS, 0, count);
                }
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
        glUniform1i(glGetUniformLocation(quadProgram, "colormap_idx"), colormap_idx);

        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, accTex);
        glUniform1i(glGetUniformLocation(quadProgram, "screenTexture"), 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        if (show_ui) {
            glUseProgram(uiProgram);
            glUniform1i(glGetUniformLocation(uiProgram, "colormap_idx"), colormap_idx);
            glBindVertexArray(uiVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Text HUD
            vec3 textColor = {0.0f, 1.0f, 0.0f}; // Matrix Green
            char buf[128];
            const char* sys_names[] = {"Kartesisch", "Zylindrisch", "Sphärisch"};
            
            sprintf(buf, "System: %s", sys_names[coord_system_to]);
            text_renderer_render(buf, 20.0f, 40.0f, 0.6f, textColor, screenW, screenH);
            
            sprintf(buf, "Modus:  %s", cam.mode_3d ? "3D (Persp)" : "2D (Ortho)");
            text_renderer_render(buf, 20.0f, 70.0f, 0.6f, textColor, screenW, screenH);

            sprintf(buf, "Proj:   %s", proj_to ? "AN" : "AUS");
            text_renderer_render(buf, 20.0f, 100.0f, 0.6f, textColor, screenW, screenH);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    unmap_file(mf);
    free_histogram(hist);
    text_renderer_cleanup();
    if (hist_vao) glDeleteVertexArrays(1, &hist_vao);
    if (hist_vbo) glDeleteBuffers(1, &hist_vbo);
    glfwTerminate();
    return 0;
}
