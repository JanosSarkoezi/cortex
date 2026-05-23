#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include "shader_utils.h"
#include "shaders_embedded.h"
#include "file_utils.h"
#include "camera.h"
#include "histogram.h"
#include "text_renderer.h"
#include "app_state.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

// ---------------------------------------------------------------------------
// Hilfsfunktionen
// ---------------------------------------------------------------------------

static void setup_fbo(GLuint* fbo, GLuint* tex, int width, int height) {
    if (*fbo) glDeleteFramebuffers(1, fbo);
    if (*tex) glDeleteTextures(1, tex);

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

static void trigger_transition(AppState* s) {
    s->morph_factor    = 0.0f;
    s->last_morph_time = glfwGetTime();
    s->needs_update    = 1;
}

// ---------------------------------------------------------------------------
// GLFW-Callbacks
// ---------------------------------------------------------------------------

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    AppState* s = glfwGetWindowUserPointer(window);
    if (width > 0 && height > 0) {
        s->width  = width;
        s->height = height;
        setup_fbo(&s->fbo, &s->acc_tex, width, height);
        s->needs_update = 1;
    }
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)xoffset;
    AppState* s = glfwGetWindowUserPointer(window);

    int ctrl_pressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)  == GLFW_PRESS ||
                       glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;

    if (ctrl_pressed) {
        if (yoffset > 0) s->offset *= 1.1f;
        else             s->offset *= 0.9f;
        if (s->offset < 0.00001f) s->offset = 0.00001f;
        if (s->offset > 10.0f)    s->offset = 10.0f;
        printf("Offset: %.6f\n", s->offset);
    } else {
        camera_zoom(&s->cam, (float)yoffset);
        s->needs_update = 1;
    }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)mods;
    AppState* s = glfwGetWindowUserPointer(window);

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            s->left_mouse_pressed = 1;
            glfwGetCursorPos(window, &s->last_x, &s->last_y);
        } else {
            s->left_mouse_pressed = 0;
        }
    }
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    AppState* s = glfwGetWindowUserPointer(window);

    if (s->left_mouse_pressed) {
        float dx = (float)(xpos - s->last_x);
        float dy = (float)(ypos - s->last_y);
        s->last_x = xpos;
        s->last_y = ypos;

        camera_rotate(&s->cam, dx, dy);
        s->needs_update = 1;
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;
    if (action != GLFW_PRESS) return;

    AppState* s = glfwGetWindowUserPointer(window);

    // --- PROGRAMM-STEUERUNG ---
    if (key == GLFW_KEY_Q) glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_H) {
        s->show_ui = !s->show_ui;
        printf("\033[1;34m[UI]\033[0m %s\n", s->show_ui ? "eingeblendet" : "ausgeblendet");
    }

    // --- MODUS-WECHSEL (2D / 3D) ---
    if (key == GLFW_KEY_SPACE) {
        s->cam.mode_3d  = !s->cam.mode_3d;
        s->needs_update = 1;
        printf("\033[1;34m[Kamera]\033[0m %s\n", s->cam.mode_3d ? "Perspektivisch" : "Orthografisch");
    }

    if (key == GLFW_KEY_P) {
        s->proj_from = s->proj_to;
        s->proj_to   = !s->proj_to;
        trigger_transition(s);
        printf("\033[1;34m[Projektion]\033[0m %s\n", s->proj_to ? "AN" : "AUS");
    }

    if (key == GLFW_KEY_1 || key == GLFW_KEY_2 || key == GLFW_KEY_3) {
        s->current_view = (key == GLFW_KEY_1) ? 0 : (key == GLFW_KEY_2 ? 1 : 2);
        camera_set_view(&s->cam, s->current_view);

        if (s->proj_to) {
            s->proj_from = s->proj_to;
            trigger_transition(s);
        } else {
            s->needs_update = 1;
        }
    }

    // --- KOORDINATENSYSTEME ---
    if (key == GLFW_KEY_K || key == GLFW_KEY_Z || key == GLFW_KEY_S) {
        int target_sys = (key == GLFW_KEY_K) ? 0 : (key == GLFW_KEY_Z ? 1 : 2);

        if (s->coord_system_to != target_sys) {
            s->coord_system_from = s->coord_system_to;
            s->coord_system_to   = target_sys;
            s->proj_from         = s->proj_to;
            trigger_transition(s);

            const char* names[] = {"Kartesisch", "Zylindrisch", "Sphärisch"};
            printf("\033[1;34m[System]\033[0m %s -> %s\n",
                   names[s->coord_system_from], names[s->coord_system_to]);
        }
    }

    // --- KAMERA & DARSTELLUNG ---
    if (key == GLFW_KEY_R) {
        camera_reset(&s->cam);
        trigger_transition(s);
        printf("\033[1;34m[Reset]\033[0m Kamera zurückgesetzt.\n");
    }

    if (key == GLFW_KEY_TAB) {
        s->colormap_idx = (s->colormap_idx + 1) % 3;
        const char* names[] = {"Matrix", "Turbo", "Viridis"};
        printf("\033[1;34m[Farbe]\033[0m Colormap: %s\n", names[s->colormap_idx]);
        s->needs_update = 1;
    }

    if (key == GLFW_KEY_M) {
        if (s->hist.points != NULL) {
            s->use_histogram = !s->use_histogram;
            s->needs_update  = 1;
            printf("\033[1;34m[Modus]\033[0m %s\n",
                   s->use_histogram ? "3D-Histogramm (Voxel)" : "Sequenziell (Rohdaten)");
        } else {
            printf("\033[1;31m[Fehler]\033[0m Histogramm nicht verfügbar.\n");
        }
    }

    if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) {
        s->point_size   = (s->point_size < 4.0f) ? s->point_size + 0.5f : 4.0f;
        s->needs_update = 1;
    }
    if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) {
        s->point_size   = (s->point_size > 1.0f) ? s->point_size - 0.5f : 1.0f;
        s->needs_update = 1;
    }
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char** argv) {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // State initialisieren (alle Felder auf Null/Default)
    AppState state = {
        .width          = 1024,
        .height         = 1024,
        .needs_update   = 1,
        .exposure       = 1.0f,
        .offset         = 0.5f,
        .point_size     = 1.0f,
        .colormap_idx   = 0,
        .use_histogram  = 0,
        .morph_factor   = 1.0f,
        .morph_duration = 2.0f,
        .show_ui        = 1,
        .hist           = {NULL, 0, 0},
    };
    camera_init(&state.cam);

    GLFWwindow* window = glfwCreateWindow(state.width, state.height,
                                          "Cortex - Binary Visualizer", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // State-Pointer ins Fenster hängen → Callbacks können ihn abrufen
    glfwSetWindowUserPointer(window, &state);

    glfwSetScrollCallback(window,       scroll_callback);
    glfwSetKeyCallback(window,          key_callback);
    glfwSetMouseButtonCallback(window,  mouse_button_callback);
    glfwSetCursorPosCallback(window,    cursor_pos_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    state.acc_program  = create_shader_program_from_source(core_default_vert_source, core_default_frag_source);
    state.quad_program = create_shader_program_from_source(quad_quad_vert_source,    quad_quad_frag_source);
    state.ui_program   = create_shader_program_from_source(ui_ui_vert_source,        ui_ui_frag_source);

    if (!state.acc_program || !state.quad_program || !state.ui_program) {
        fprintf(stderr, "[Fehler] Shader-Kompilierung fehlgeschlagen.\n");
        glfwTerminate();
        return -1;
    }

    // Uniform Locations einmalig cachen
    state.uloc_mvp        = glGetUniformLocation(state.acc_program, "mvp");
    state.uloc_cs_from    = glGetUniformLocation(state.acc_program, "coord_system_from");
    state.uloc_cs_to      = glGetUniformLocation(state.acc_program, "coord_system_to");
    state.uloc_morph      = glGetUniformLocation(state.acc_program, "morph_factor");
    state.uloc_proj_view  = glGetUniformLocation(state.acc_program, "projection_view");
    state.uloc_point_size = glGetUniformLocation(state.acc_program, "u_point_size");
    state.uloc_proj_from  = glGetUniformLocation(state.acc_program, "u_proj_from");
    state.uloc_proj_to    = glGetUniformLocation(state.acc_program, "u_proj_to");
    state.uloc_use_hist   = glGetUniformLocation(state.acc_program, "u_use_histogram");
    state.uloc_raw_data   = glGetUniformLocation(state.acc_program, "raw_data");

    state.uloc_q_exposure   = glGetUniformLocation(state.quad_program, "exposure");
    state.uloc_q_offset     = glGetUniformLocation(state.quad_program, "offset");
    state.uloc_q_colormap   = glGetUniformLocation(state.quad_program, "colormap_idx");
    state.uloc_q_screen_tex = glGetUniformLocation(state.quad_program, "screenTexture");

    state.uloc_ui_colormap  = glGetUniformLocation(state.ui_program, "colormap_idx");

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

    // Histogramm berechnen
    state.hist = calculate_histogram(mf.data, mf.size);
    if (state.hist.points) {
        glGenVertexArrays(1, &state.hist_vao);
        glGenBuffers(1, &state.hist_vbo);
        glBindVertexArray(state.hist_vao);
        glBindBuffer(GL_ARRAY_BUFFER, state.hist_vbo);
        glBufferData(GL_ARRAY_BUFFER, state.hist.num_points * sizeof(HistogramPoint),
                     state.hist.points, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_FALSE,
                              sizeof(HistogramPoint), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 1, GL_UNSIGNED_INT, GL_FALSE,
                              sizeof(HistogramPoint), (void*)offsetof(HistogramPoint, count));

        if (mf.size > 20 * 1024 * 1024) {
            state.use_histogram = 1;
            printf("\033[1;34m[Cortex]\033[0m Große Datei erkannt, Histogramm-Modus aktiviert.\n");
        }
    }

    // TBO für Rohdaten
    glGenBuffers(1, &state.tbo_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, state.tbo_buffer);
    glBufferData(GL_TEXTURE_BUFFER, mf.size, mf.data, GL_STATIC_DRAW);

    glGenTextures(1, &state.tbo_tex);
    glBindTexture(GL_TEXTURE_BUFFER, state.tbo_tex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R8UI, state.tbo_buffer);

    // Leeres VAO für TBO-Rendering
    glGenVertexArrays(1, &state.vao);

    // Fullscreen-Quad
    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    glGenVertexArrays(1, &state.quad_vao);
    glGenBuffers(1, &state.quad_vbo);
    glBindVertexArray(state.quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, state.quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // UI-Skalen-Quad (rechter Rand)
    float uiVertices[] = {
        0.85f,  0.8f,  0.0f, 1.0f,
        0.85f, -0.8f,  0.0f, 0.0f,
        0.95f, -0.8f,  1.0f, 0.0f,

        0.85f,  0.8f,  0.0f, 1.0f,
        0.95f, -0.8f,  1.0f, 0.0f,
        0.95f,  0.8f,  1.0f, 1.0f
    };
    glGenVertexArrays(1, &state.ui_vao);
    glGenBuffers(1, &state.ui_vbo);
    glBindVertexArray(state.ui_vao);
    glBindBuffer(GL_ARRAY_BUFFER, state.ui_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uiVertices), &uiVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    setup_fbo(&state.fbo, &state.acc_tex, state.width, state.height);

    glEnable(GL_PROGRAM_POINT_SIZE);
    text_renderer_init();

    state.last_frame_time = glfwGetTime();

    // -----------------------------------------------------------------------
    // Render-Loop
    // -----------------------------------------------------------------------
    while (!glfwWindowShouldClose(window)) {
        double current_time = glfwGetTime();
        float dt = (float)(current_time - state.last_frame_time);
        state.last_frame_time = current_time;

        if (state.morph_factor < 1.0f) {
            state.morph_factor += dt / state.morph_duration;
            if (state.morph_factor >= 1.0f) {
                state.morph_factor       = 1.0f;
                state.coord_system_from  = state.coord_system_to;
            }
            state.needs_update = 1;
        }

        if (state.cam.anim_factor < 1.0f) {
            state.cam.anim_factor += dt / 0.5f;
            if (state.cam.anim_factor >= 1.0f) {
                state.cam.anim_factor = 1.0f;
                glm_quat_copy(state.cam.target_orientation, state.cam.orientation);
            }
            state.needs_update = 1;
        }

        if (state.needs_update) {
            int screenW, screenH;
            glfwGetFramebufferSize(window, &screenW, &screenH);
            state.cam.aspect = (float)screenW / (float)screenH;

            mat4 mvp;
            camera_get_mvp(&state.cam, mvp);

            glBindFramebuffer(GL_FRAMEBUFFER, state.fbo);
            glViewport(0, 0, state.width, state.height);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);

            glUseProgram(state.acc_program);
            glUniformMatrix4fv(state.uloc_mvp,        1, GL_FALSE, (float*)mvp);
            glUniform1i(state.uloc_cs_from,    state.coord_system_from);
            glUniform1i(state.uloc_cs_to,      state.coord_system_to);
            glUniform1f(state.uloc_morph,      state.morph_factor);
            glUniform1i(state.uloc_proj_view,  state.current_view);
            glUniform1f(state.uloc_point_size, state.point_size);
            glUniform1i(state.uloc_proj_from,  state.proj_from);
            glUniform1i(state.uloc_proj_to,    state.proj_to);
            glUniform1i(state.uloc_use_hist,   state.use_histogram);

            if (state.use_histogram) {
                glBindVertexArray(state.hist_vao);
                glDrawArrays(GL_POINTS, 0, (GLsizei)state.hist.num_points);
            } else {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_BUFFER, state.tbo_tex);
                glUniform1i(state.uloc_raw_data, 0);

                glBindVertexArray(state.vao);
                GLsizei count = (mf.size > 2) ? (GLsizei)(mf.size - 2) : 0;
                if (count > 0) {
                    glDrawArrays(GL_POINTS, 0, count);
                }
            }

            glDisable(GL_BLEND);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            state.needs_update = 0;
        }

        int screenW, screenH;
        glfwGetFramebufferSize(window, &screenW, &screenH);
        glViewport(0, 0, screenW, screenH);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(state.quad_program);
        glUniform1f(state.uloc_q_exposure,  state.exposure);
        glUniform1f(state.uloc_q_offset,    state.offset);
        glUniform1i(state.uloc_q_colormap,  state.colormap_idx);

        glBindVertexArray(state.quad_vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, state.acc_tex);
        glUniform1i(state.uloc_q_screen_tex, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        if (state.show_ui) {
            glUseProgram(state.ui_program);
            glUniform1i(state.uloc_ui_colormap, state.colormap_idx);
            glBindVertexArray(state.ui_vao);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            vec3 textColor = {0.0f, 1.0f, 0.0f};
            char buf[128];
            const char* sys_names[] = {"Kartesisch", "Zylindrisch", "Sphärisch"};

            snprintf(buf, sizeof(buf), "System: %s", sys_names[state.coord_system_to]);
            text_renderer_render(buf, 20.0f, 40.0f, 0.6f, textColor, screenW, screenH);

            snprintf(buf, sizeof(buf), "Modus:  %s", state.cam.mode_3d ? "3D (Persp)" : "2D (Ortho)");
            text_renderer_render(buf, 20.0f, 70.0f, 0.6f, textColor, screenW, screenH);

            snprintf(buf, sizeof(buf), "Proj:   %s", state.proj_to ? "AN" : "AUS");
            text_renderer_render(buf, 20.0f, 100.0f, 0.6f, textColor, screenW, screenH);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // -----------------------------------------------------------------------
    // Cleanup
    // -----------------------------------------------------------------------
    unmap_file(mf);
    free_histogram(state.hist);
    text_renderer_cleanup();

    if (state.hist_vao) glDeleteVertexArrays(1, &state.hist_vao);
    if (state.hist_vbo) glDeleteBuffers(1,    &state.hist_vbo);

    glDeleteVertexArrays(1, &state.vao);
    glDeleteVertexArrays(1, &state.quad_vao);
    glDeleteBuffers(1,      &state.quad_vbo);
    glDeleteVertexArrays(1, &state.ui_vao);
    glDeleteBuffers(1,      &state.ui_vbo);
    glDeleteBuffers(1,      &state.tbo_buffer);
    glDeleteTextures(1,     &state.tbo_tex);

    if (state.fbo)     glDeleteFramebuffers(1, &state.fbo);
    if (state.acc_tex) glDeleteTextures(1,     &state.acc_tex);

    glDeleteProgram(state.acc_program);
    glDeleteProgram(state.quad_program);
    glDeleteProgram(state.ui_program);

    glfwTerminate();
    return 0;
}
