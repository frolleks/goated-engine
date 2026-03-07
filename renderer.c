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

bool draw_mesh_wireframe(SDL_Renderer *renderer, const Mesh *mesh, float angle,
                         float camera_distance, int width, int height) {
    SDL_FPoint *projected_vertices =
        SDL_malloc(sizeof(*projected_vertices) * mesh->vertex_count);
    if (!projected_vertices) {
        SDL_Log("Out of memory");
        return SDL_APP_FAILURE;
    }

    if (!SDL_GetRenderOutputSize(renderer, &width, &height)) {
        SDL_Log("Couldn't get renderer size: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // /* Vertices are in centered normalized space: (0, 0) is screen center. */
    // triangle[0] = (SDL_Vertex){normalize_coords(0.0f, 0.0f, width, height),
    // green, {0.0f, 0.0f}}; triangle[1] = (SDL_Vertex){normalize_coords(0.25f,
    // -0.5f, width, height), green, {0.0f, 0.0f}}; triangle[2] =
    // (SDL_Vertex){normalize_coords(-0.25f, -0.5f, width, height), green,
    // {0.0f, 0.0f}};

    if (!SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE)) {
        SDL_Log("Couldn't set the clear color: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_RenderClear(renderer)) {
        SDL_Log("Couldn't clear the frame: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // if (!SDL_RenderGeometry(renderer, NULL, triangle, 3, NULL, 0))
    // {
    //     SDL_Log("Couldn't draw the triangle: %s", SDL_GetError());
    //     return SDL_APP_FAILURE;
    // }

    if (!SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE)) {
        SDL_Log("Couldn't set the clear color: %s", SDL_GetError());
        return SDL_APP_FAILURE;
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

        // if (!SDL_RenderFillRect(renderer, &rect)) {
        //     SDL_Log("Couldn't draw the rectangle: %s", SDL_GetError());
        //     return SDL_APP_FAILURE;
        // }
    }

    for (size_t i = 0; i < mesh->triangle_count; ++i) {
        Triangle t = mesh->triangles[i];

        SDL_FPoint a = projected_vertices[t.v1];
        SDL_FPoint b = projected_vertices[t.v2];
        SDL_FPoint c = projected_vertices[t.v3];

        if (!render_triangle_outline(renderer, a, b, c)) {
            SDL_free(projected_vertices);
            SDL_Log("Couldn't draw the triangle outline: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }
    }

    if (!SDL_RenderPresent(renderer)) {
        SDL_Log("Couldn't present the frame: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_free(projected_vertices);

    return true;
}