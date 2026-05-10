#include "camera.h"
#include <math.h>

void camera_init(Camera* cam) {
    glm_quat_identity(cam->orientation);
    cam->distance = 2.5f;
    cam->aspect = 1.0f;
    cam->mode_3d = 0;
}

void camera_get_mvp(Camera* cam, mat4 mvp) {
    mat4 projection, view;

    if (cam->mode_3d) {
        glm_perspective(glm_rad(45.0f), cam->aspect, 0.1f, 100.0f, projection);
    } else {
        float scale = cam->distance * 0.5f;
        glm_ortho(-scale * cam->aspect, scale * cam->aspect, -scale, scale, -10.0f, 10.0f, projection);
    }

    // View Matrix mit Quaternions
    glm_mat4_identity(view);
    
    // 1. In die Ferne rücken (Z-Achse)
    glm_translate_z(view, -cam->distance);
    
    // 2. Rotation anwenden
    mat4 rotation_mat;
    glm_quat_mat4(cam->orientation, rotation_mat);
    glm_mat4_mul(view, rotation_mat, view);

    glm_mat4_mul(projection, view, mvp);
}

void camera_rotate(Camera* cam, float dx, float dy) {
    // Empfindlichkeit
    float sensitivity = 0.005f;

    versor q_x, q_y;
    vec3 axis_x = {1.0f, 0.0f, 0.0f};
    vec3 axis_y = {0.0f, 1.0f, 0.0f};

    // Rotation um die X-Achse (Pitch) - lokal
    glm_quatv(q_x, dy * sensitivity, axis_x);
    // Rotation um die Y-Achse (Yaw) - global (für natürlicheres orbitales Gefühl)
    glm_quatv(q_y, dx * sensitivity, axis_y);

    // Multiplikation: q_x * orientation * q_y
    // q_x von links = lokale Rotation
    // q_y von rechts = globale Rotation
    glm_quat_mul(q_x, cam->orientation, cam->orientation);
    glm_quat_mul(cam->orientation, q_y, cam->orientation);

    glm_quat_normalize(cam->orientation);
}

void camera_zoom(Camera* cam, float delta) {
    if (delta > 0) cam->distance *= 0.9f;
    else cam->distance *= 1.1f;

    if (cam->distance < 0.1f) cam->distance = 0.1f;
    if (cam->distance > 20.0f) cam->distance = 20.0f;
}

void camera_reset(Camera* cam) {
    glm_quat_identity(cam->orientation);
    cam->distance = 2.5f;
}
