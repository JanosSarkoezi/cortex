#include "camera.h"
#include <math.h>

void camera_init(Camera* cam) {
    glm_quat_identity(cam->orientation);
    glm_quat_identity(cam->start_orientation);
    glm_quat_identity(cam->target_orientation);
    glm_vec3_zero(cam->look_at);
    glm_vec3_zero(cam->start_look_at);
    glm_vec3_zero(cam->target_look_at);
    cam->anim_factor = 1.0f;
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

    // 2. Rotation anwenden (mit Slerp falls Animation läuft)
    versor current_orient;
    vec3 current_look_at;

    if (cam->anim_factor < 1.0f) {
        glm_quat_slerp(cam->start_orientation, cam->target_orientation, cam->anim_factor, current_orient);
        glm_quat_copy(current_orient, cam->orientation);
        glm_vec3_lerp(cam->start_look_at, cam->target_look_at, cam->anim_factor, current_look_at);
        glm_vec3_copy(current_look_at, cam->look_at);
    } else {
        glm_quat_copy(cam->orientation, current_orient);
        glm_vec3_copy(cam->look_at, current_look_at);
    }

    mat4 rotation_mat;
    glm_quat_mat4(current_orient, rotation_mat);
    glm_mat4_mul(view, rotation_mat, view);

    // 3. Den Fokuspunkt verschieben (Translation in die entgegengesetzte Richtung)
    vec3 neg_look_at;
    glm_vec3_negate_to(current_look_at, neg_look_at);
    glm_translate(view, neg_look_at);

    glm_mat4_mul(projection, view, mvp);
}

void camera_rotate(Camera* cam, float dx, float dy) {
    // Wenn eine Animation läuft, brechen wir sie ab
    cam->anim_factor = 1.0f;

    // Empfindlichkeit
    float sensitivity = 0.005f;

    versor q_total;
    versor q_x, q_y;

    vec3 axis_x = {1.0f, 0.0f, 0.0f};
    vec3 axis_y = {0.0f, 1.0f, 0.0f};

    // Erstelle Quaternions für die aktuellen Mausbewegungen
    glm_quatv(q_x, dy * sensitivity, axis_x);
    glm_quatv(q_y, dx * sensitivity, axis_y);

    glm_quat_mul(q_x, q_y, q_total);
    glm_quat_mul(q_total, cam->orientation, cam->orientation);

    // Normalisierung gegen Drift
    glm_quat_normalize(cam->orientation);
}

void camera_zoom(Camera* cam, float delta) {
    if (delta > 0) cam->distance *= 0.9f;
    else cam->distance *= 1.1f;

    if (cam->distance < 0.1f) cam->distance = 0.1f;
    if (cam->distance > 20.0f) cam->distance = 20.0f;
}

void camera_zoom_to_mouse(Camera* cam, float delta, float mouse_x, float mouse_y, int screen_w, int screen_h) {
    mat4 mvp, inv_mvp;
    camera_get_mvp(cam, mvp);
    glm_mat4_inv(mvp, inv_mvp);

    // Maus in Normalized Device Coordinates (NDC) [-1, 1]
    float nx = (2.0f * mouse_x) / screen_w - 1.0f;
    float ny = 1.0f - (2.0f * mouse_y) / screen_h;

    // Wir brauchen den Punkt im World Space.
    // Da wir keine Oberfläche haben, nehmen wir den Punkt auf der Ebene des aktuellen 'look_at'.
    // Wir berechnen den Strahl von der Kamera durch die Mausposition.
    vec4 near_pt = {nx, ny, -1.0f, 1.0f};
    vec4 far_pt  = {nx, ny,  1.0f, 1.0f};

    glm_mat4_mulv(inv_mvp, near_pt, near_pt);
    glm_mat4_mulv(inv_mvp, far_pt,  far_pt);

    glm_vec4_scale(near_pt, 1.0f / near_pt[3], near_pt);
    glm_vec4_scale(far_pt,  1.0f / far_pt[3],  far_pt);

    // Strahl-Richtung
    vec3 dir;
    glm_vec3_sub((vec3){far_pt[0], far_pt[1], far_pt[2]}, (vec3){near_pt[0], near_pt[1], near_pt[2]}, dir);
    glm_vec3_normalize(dir);

    // Wir suchen den Schnittpunkt mit der Ebene, die durch 'look_at' geht und orthogonal zur Kamera-Blickrichtung ist.
    // Kamera-Position berechnen
    mat4 rot;
    glm_quat_mat4(cam->orientation, rot);
    vec3 forward = {0.0f, 0.0f, -1.0f};
    glm_mat4_mulv3(rot, forward, 1.0f, forward); // Vorwärtsvektor im Weltraum (invertiert, da View-Matrix)
    
    // In der Arcball-Logik schaut die Kamera von (0,0,distance) gedreht auf (look_at).
    // Einfacher: Wir nutzen den NDC-Punkt und projizieren ihn auf die Ebene.
    // Bei Ortho ist es trivial, bei Perspektive müssen wir die Tiefe beachten.
    
    // Korrekte Kameraposition: R * (0,0,dist) + look_at
    vec3 cam_pos;
    vec3 local_pos = {0.0f, 0.0f, cam->distance};
    glm_mat4_mulv3(rot, local_pos, 1.0f, cam_pos);
    glm_vec3_add(cam_pos, cam->look_at, cam_pos);

    // Schnittpunkt mit Ebene: (P - LookAt) . Normal = 0
    // Normal ist der Vektor von LookAt zur Kamera (da die Kamera immer auf LookAt schaut)
    vec3 n;
    glm_vec3_sub(cam_pos, cam->look_at, n);
    glm_vec3_normalize(n);

    float d = glm_vec3_dot(n, cam->look_at);
    float t = (d - glm_vec3_dot(n, (vec3){near_pt[0], near_pt[1], near_pt[2]})) / glm_vec3_dot(n, dir);
    
    vec3 world_mouse;
    glm_vec3_scale(dir, t, world_mouse);
    glm_vec3_add(world_mouse, (vec3){near_pt[0], near_pt[1], near_pt[2]}, world_mouse);

    // Zoomfaktor
    float zoom_factor = (delta > 0) ? 0.9f : 1.1f;
    float old_dist = cam->distance;
    cam->distance *= zoom_factor;
    
    if (cam->distance < 0.1f) cam->distance = 0.1f;
    if (cam->distance > 20.0f) cam->distance = 20.0f;

    // Das neue look_at berechnen, damit world_mouse an der gleichen Stelle bleibt
    // LookAt_neu = WorldMouse - (WorldMouse - LookAt_alt) * (Dist_neu / Dist_alt)
    vec3 diff;
    glm_vec3_sub(world_mouse, cam->look_at, diff);
    glm_vec3_scale(diff, cam->distance / old_dist, diff);
    glm_vec3_sub(world_mouse, diff, cam->look_at);
}

void camera_reset(Camera* cam) {
    glm_quat_copy(cam->orientation, cam->start_orientation);
    glm_quat_identity(cam->target_orientation);
    
    glm_vec3_copy(cam->look_at, cam->start_look_at);
    glm_vec3_zero(cam->target_look_at);
    
    cam->anim_factor = 0.0f;
    cam->distance = 2.5f;
}

void camera_set_view(Camera* cam, int plane) {
    glm_quat_copy(cam->orientation, cam->start_orientation);
    glm_quat_identity(cam->target_orientation);
    
    glm_vec3_copy(cam->look_at, cam->start_look_at);
    glm_vec3_zero(cam->target_look_at); // Wir zentrieren die Ansicht beim Wechsel
    
    versor q;
    vec3 axis_y = {0.0f, 1.0f, 0.0f};
    vec3 axis_x = {1.0f, 0.0f, 0.0f};

    if (plane == 1) { // YZ
        glm_quatv(q, glm_rad(90.0f), axis_y);
        glm_quat_mul(q, cam->target_orientation, cam->target_orientation);
    } else if (plane == 2) { // ZX
        glm_quatv(q, glm_rad(90.0f), axis_x);
        glm_quat_mul(q, cam->target_orientation, cam->target_orientation);
    }
    
    cam->anim_factor = 0.0f;
}
