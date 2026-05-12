#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <glad/glad.h>
#include <cglm/cglm.h>

void text_renderer_init();
void text_renderer_render(const char* text, float x, float y, float scale, vec3 color, int screen_width, int screen_height);
void text_renderer_cleanup();

#endif
