#include "renderer.h"

SDL_FPoint project_to_2d(float x, float y, float z) {
    return (SDL_FPoint){x / z, y / z};
}

bool render_triangle_outline(SDL_Renderer *renderer, SDL_FPoint a, SDL_FPoint b,
                             SDL_FPoint c) {
    return SDL_RenderLine(renderer, a.x, a.y, b.x, b.y) &&
           SDL_RenderLine(renderer, b.x, b.y, c.x, c.y) &&
           SDL_RenderLine(renderer, c.x, c.y, a.x, a.y);
}