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

bool draw_mesh_wireframe(SDL_Renderer *renderer, const Mesh *mesh,
                         SDL_FPoint *projected_vertices, float angle,
                         float camera_distance) {
    int width = 0;
    int height = 0;

    if (!renderer || !mesh || !projected_vertices) {
        return false;
    }

    if (!SDL_GetRenderOutputSize(renderer, &width, &height)) {
        SDL_Log("Couldn't get renderer size: %s", SDL_GetError());
        return false;
    }

    if (!SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE)) {
        SDL_Log("Couldn't set the clear color: %s", SDL_GetError());
        return false;
    }

    if (!SDL_RenderClear(renderer)) {
        SDL_Log("Couldn't clear the frame: %s", SDL_GetError());
        return false;
    }

    if (!SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE)) {
        SDL_Log("Couldn't set draw color: %s", SDL_GetError());
        return false;
    }

    for (size_t i = 0; i < mesh->vertex_count; ++i) {
        Vertex rotated = rotate_y(mesh->vertices[i], angle);
        rotated.z += camera_distance;

        if (rotated.z <= 0.01f) {
            projected_vertices[i] = (SDL_FPoint){0.0f, 0.0f};
            continue;
        }

        SDL_FPoint p = project_to_2d(rotated.x, rotated.y, rotated.z);
        projected_vertices[i] = normalize_coords(p.x, p.y, width, height);
    }

    for (size_t i = 0; i < mesh->triangle_count; ++i) {
        Triangle t = mesh->triangles[i];

        SDL_FPoint a = projected_vertices[t.v1];
        SDL_FPoint b = projected_vertices[t.v2];
        SDL_FPoint c = projected_vertices[t.v3];

        if (!render_triangle_outline(renderer, a, b, c)) {
            SDL_Log("Couldn't draw triangle outline: %s", SDL_GetError());
            return false;
        }
    }

    if (!SDL_RenderPresent(renderer)) {
        SDL_Log("Couldn't present the frame: %s", SDL_GetError());
        return false;
    }

    return true;
}