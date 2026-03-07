#ifndef OBJECT_H
#define OBJECT_H

struct VerticesStruct {
    float x;
    float y;
    float z;
};

struct Edge {
    int from;
    int to;
};

struct Triangle {
    int v1;
    int v2;
    int v3;
};

SDL_FPoint project_to_2d(float x, float y, float z);
struct VerticesStruct rotate_xz(float x, float y, float z, float angle);
bool render_triangle_outline(SDL_Renderer *renderer, SDL_FPoint a, SDL_FPoint b,
                             SDL_FPoint c);

#endif