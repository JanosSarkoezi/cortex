#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>

typedef struct {
    versor orientation;
    versor start_orientation;
    versor target_orientation;
    float anim_factor; // 1.0 wenn keine Animation läuft
    float distance;
    float aspect;
    int mode_3d;
} Camera;

void camera_init(Camera* cam);
void camera_get_mvp(Camera* cam, mat4 mvp);
void camera_rotate(Camera* cam, float dx, float dy);
void camera_zoom(Camera* cam, float delta);
void camera_reset(Camera* cam);
void camera_set_view(Camera* cam, int plane); // 0: XY, 1: YZ, 2: ZX

#endif
