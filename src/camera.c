#include "camera.h"
#include <math.h>

void camera_init(Camera* cam) {
    cam->yaw = 0.0f;
    cam->pitch = 0.0f;
    cam->distance = 2.5f;
    cam->aspect = 1.0f;
    cam->mode_3d = 0;
}

void camera_get_mvp(Camera* cam, mat4 mvp) {
    mat4 projection, view;

    if (cam->mode_3d) {
        glm_perspective(glm_rad(45.0f), cam->aspect, 0.1f, 100.0f, projection);
    } else {
        // In 2D nutzen wir Ortho, aber lassen den Zoom (distance) einfließen
        float scale = cam->distance * 0.5f;
        glm_ortho(-scale * cam->aspect, scale * cam->aspect, -scale, scale, -10.0f, 10.0f, projection);
    }

    // View Matrix (Orbital)
    vec3 eye;
    eye[0] = cam->distance * cos(glm_rad(cam->pitch)) * sin(glm_rad(cam->yaw));
    eye[1] = cam->distance * sin(glm_rad(cam->pitch));
    eye[2] = cam->distance * cos(glm_rad(cam->pitch)) * cos(glm_rad(cam->yaw));

    vec3 center = {0.0f, 0.0f, 0.0f};
    vec3 up = {0.0f, 1.0f, 0.0f};
    glm_lookat(eye, center, up, view);

    glm_mat4_mul(projection, view, mvp);
}

void camera_rotate(Camera* cam, float dx, float dy) {
    cam->yaw -= dx * 0.5f;
    cam->pitch += dy * 0.5f;

    if (cam->pitch > 89.0f) cam->pitch = 89.0f;
    if (cam->pitch < -89.0f) cam->pitch = -89.0f;
}

void camera_zoom(Camera* cam, float delta) {
    if (delta > 0) cam->distance *= 0.9f;
    else cam->distance *= 1.1f;

    if (cam->distance < 0.1f) cam->distance = 0.1f;
    if (cam->distance > 20.0f) cam->distance = 20.0f;
}

void camera_reset(Camera* cam) {
    cam->yaw = 0.0f;
    cam->pitch = 0.0f;
    cam->distance = 2.5f;
}
