#ifndef APP_STATE_H
#define APP_STATE_H

#include <glad/glad.h>
#include "camera.h"
#include "histogram.h"

typedef struct {
    // --- Fenster ---
    int width;
    int height;
    int needs_update;

    // --- GL: Accumulation-Framebuffer ---
    GLuint fbo;
    GLuint acc_tex;

    // --- GL: Histogramm ---
    GLuint hist_vao;
    GLuint hist_vbo;

    // --- GL: Rohdaten (Texture Buffer Object) ---
    GLuint tbo_buffer;
    GLuint tbo_tex;

    // --- GL: Geometrie ---
    GLuint vao;         // leeres VAO für TBO-Rendering
    GLuint quad_vao;
    GLuint quad_vbo;
    GLuint ui_vao;
    GLuint ui_vbo;

    // --- GL: Shader-Programme ---
    GLuint acc_program;
    GLuint quad_program;
    GLuint ui_program;

    // --- Gecachte Uniform Locations: acc_program ---
    GLint uloc_mvp;
    GLint uloc_cs_from;
    GLint uloc_cs_to;
    GLint uloc_morph;
    GLint uloc_proj_view;
    GLint uloc_point_size;
    GLint uloc_proj_from;
    GLint uloc_proj_to;
    GLint uloc_use_hist;
    GLint uloc_raw_data;

    // --- Gecachte Uniform Locations: quad_program ---
    GLint uloc_q_exposure;
    GLint uloc_q_offset;
    GLint uloc_q_colormap;
    GLint uloc_q_screen_tex;

    // --- Gecachte Uniform Locations: ui_program ---
    GLint uloc_ui_colormap;

    // --- Render-Einstellungen ---
    float exposure;
    float offset;
    float point_size;
    int   colormap_idx;  // 0: Matrix, 1: Turbo, 2: Viridis
    int   use_histogram;

    // --- Koordinatensystem & Projektion ---
    int coord_system_from;
    int coord_system_to;
    int proj_from;
    int proj_to;
    int current_view;

    // --- Morphing-Animation ---
    float  morph_factor;
    float  morph_duration;
    double last_morph_time;
    double last_frame_time;

    // --- UI ---
    int show_ui;

    // --- Kamera ---
    Camera cam;

    // --- Maus ---
    double last_x;
    double last_y;
    int    left_mouse_pressed;

    // --- Datei-Daten ---
    Histogram hist;
} AppState;

#endif // APP_STATE_H
