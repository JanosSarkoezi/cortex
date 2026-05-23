#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>

typedef struct {
    versor orientation;
    versor start_orientation;
    versor target_orientation;
    vec3 look_at;      // Punkt, auf den die Kamera schaut
    vec3 start_look_at;  // Für Animationen
    vec3 target_look_at; // Für Animationen
    float anim_factor; // 1.0 wenn keine Animation läuft
    float distance;
    float aspect;
    int mode_3d;
} Camera;

void camera_init(Camera* cam);
void camera_get_mvp(Camera* cam, mat4 mvp);
void camera_rotate(Camera* cam, float dx, float dy);
void camera_zoom(Camera* cam, float delta);
void camera_zoom_to_mouse(Camera* cam, float delta, float mouse_x, float mouse_y, int screen_w, int screen_h);
void camera_reset(Camera* cam);
void camera_set_view(Camera* cam, int plane); // 0: XY, 1: YZ, 2: ZX

#endif
