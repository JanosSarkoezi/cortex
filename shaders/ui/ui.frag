#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform int colormap_idx;

vec3 turbo(float x) {
    const vec4 kRedVec4 = vec4(0.13572138, 4.61539260, -42.66032258, 132.13108234);
    const vec4 kGreenVec4 = vec4(0.09140261, 2.19418839, 4.84296658, -14.18503344);
    const vec4 kBlueVec4 = vec4(0.10667330, 12.64194608, -60.58204836, 110.36276771);
    const vec2 kRedVec2 = vec2(-152.27602325, 63.66285895);
    const vec2 kGreenVec2 = vec2(4.27729857, 13.29750586);
    const vec2 kBlueVec2 = vec2(-89.92745637, 27.34894973);
    x = clamp(x, 0.0, 1.0);
    vec4 v4 = vec4(1.0, x, x * x, x * x * x);
    vec2 v2 = v4.zw * v4.z;
    return vec3(
        dot(v4, kRedVec4) + dot(v2, kRedVec2),
        dot(v4, kGreenVec4) + dot(v2, kGreenVec2),
        dot(v4, kBlueVec4) + dot(v2, kBlueVec2)
    );
}

vec3 viridis(float t) {
    const vec3 c0 = vec3(0.2777273272234177, 0.005407344544567191, 0.33409912377443837);
    const vec3 c1 = vec3(0.1050930431085774, 1.4046135298999011, 1.384590162594685);
    const vec3 c2 = vec3(-0.3308618287255563, 0.21444755381331413, 0.09509516302826005);
    const vec3 c3 = vec3(-4.634230498983486, -5.799100973351585, -19.33244095627987);
    const vec3 c4 = vec3(6.22826993623691, 14.179933313019077, 56.69055260068105);
    const vec3 c5 = vec3(4.77638499433258, -13.745145377743066, -65.35303263337251);
    const vec3 c6 = vec3(-5.435455855934631, 4.645852612178535, 26.31243465957842);
    t = clamp(t, 0.0, 1.0);
    return c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * (c5 + t * c6)))));
}

void main() {
    float t = TexCoord.y; // Vertikaler Gradient
    vec3 color;
    if (colormap_idx == 0) {
        vec3 lowColor = vec3(0.0, 0.2, 0.05);
        vec3 highColor = vec3(0.2, 1.0, 0.5);
        color = mix(lowColor, highColor, t);
    } else if (colormap_idx == 1) {
        color = turbo(t);
    } else {
        color = viridis(t);
    }
    
    // Rand hinzufügen
    float border = step(0.02, TexCoord.x) * step(TexCoord.x, 0.98) * step(0.01, TexCoord.y) * step(TexCoord.y, 0.99);
    FragColor = vec4(color * border + vec3(0.5) * (1.0 - border), 1.0);
}
