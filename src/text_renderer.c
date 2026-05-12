#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include "text_renderer.h"
#include "shaders_embedded.h"
#include "shader_utils.h"
#include "font_embedded.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GLuint font_vao, font_vbo;
static GLuint font_texture;
static GLuint text_program;
static stbtt_bakedchar cdata[96]; // ASCII 32..126

#define BITMAP_SIZE 512
#define MAX_TEXT_VERTICES (1024 * 6) // Max 1024 characters per draw call

void text_renderer_init() {
    unsigned char temp_bitmap[BITMAP_SIZE * BITMAP_SIZE];

    // Bake font into bitmap
    stbtt_BakeFontBitmap(font_data, 0, 32.0, temp_bitmap, BITMAP_SIZE, BITMAP_SIZE, 32, 96, cdata);

    // Create texture
    glGenTextures(1, &font_texture);
    glBindTexture(GL_TEXTURE_2D, font_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, BITMAP_SIZE, BITMAP_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create shader
    text_program = create_shader_program_from_source(text_vertex_shader_source, text_fragment_shader_source);

    // Setup VAO/VBO
    glGenVertexArrays(1, &font_vao);
    glGenBuffers(1, &font_vbo);
    glBindVertexArray(font_vao);
    glBindBuffer(GL_ARRAY_BUFFER, font_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * MAX_TEXT_VERTICES, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindVertexArray(0);
}

void text_renderer_render(const char* text, float x, float y, float scale, vec3 color, int screen_width, int screen_height) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(text_program);
    
    mat4 projection;
    glm_ortho(0.0f, (float)screen_width, (float)screen_height, 0.0f, -1.0f, 1.0f, projection);
    glUniformMatrix4fv(glGetUniformLocation(text_program, "projection"), 1, GL_FALSE, (float*)projection);
    glUniform3fv(glGetUniformLocation(text_program, "textColor"), 1, color);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font_texture);
    glUniform1i(glGetUniformLocation(text_program, "text"), 0);

    glBindVertexArray(font_vao);
    glBindBuffer(GL_ARRAY_BUFFER, font_vbo);

    size_t len = strlen(text);
    if (len > 1024) len = 1024;

    float* vertices = malloc(sizeof(float) * 4 * 6 * len);
    int vertex_count = 0;

    float curr_x = x;
    float curr_y = y;

    const unsigned char* p;
    for (p = (const unsigned char*)text; *p; p++) {
        unsigned char c = *p;
        if (c >= 32 && c <= 127) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, BITMAP_SIZE, BITMAP_SIZE, c - 32, &curr_x, &curr_y, &q, 1);

            // Apply scale to positions relative to the start point (x, y)
            float vx0 = x + (q.x0 - x) * scale;
            float vy0 = y + (q.y0 - y) * scale;
            float vx1 = x + (q.x1 - x) * scale;
            float vy1 = y + (q.y1 - y) * scale;

            float v[] = {
                vx0, vy1, q.s0, q.t1,
                vx1, vy1, q.s1, q.t1,
                vx1, vy0, q.s1, q.t0,

                vx0, vy1, q.s0, q.t1,
                vx1, vy0, q.s1, q.t0,
                vx0, vy0, q.s0, q.t0
            };
            memcpy(&vertices[vertex_count * 4], v, sizeof(v));
            vertex_count += 6;
        }
    }

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 4 * vertex_count, vertices);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);

    free(vertices);
    glBindVertexArray(0);
}

void text_renderer_cleanup() {
    glDeleteTextures(1, &font_texture);
    glDeleteBuffers(1, &font_vbo);
    glDeleteVertexArrays(1, &font_vao);
    glDeleteProgram(text_program);
}
