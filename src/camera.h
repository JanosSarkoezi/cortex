#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>

typedef struct {
    versor orientation;
    float distance;
    float aspect;
    int mode_3d;
} Camera;

void camera_init(Camera* cam);
void camera_get_mvp(Camera* cam, mat4 mvp);
void camera_rotate(Camera* cam, float dx, float dy);
void camera_zoom(Camera* cam, float delta);
void camera_reset(Camera* cam);

#endif
