#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "math3d.h"
#include "mesh.h"
#include "normalize_coords.h"
#include "renderer.h"

#include "penger.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
uint64_t previous_counter = 0;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    (void)appstate;
    (void)argc;
    (void)argv;

    SDL_SetAppMetadata("Hello Triangle", "1.0", "com.frolleks.sdl-learning");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("cube", 960, 540, SDL_WINDOW_RESIZABLE,
                                     &window, &renderer)) {
        SDL_Log("Couldn't create the window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    previous_counter = SDL_GetPerformanceCounter();

    return SDL_APP_CONTINUE;
}

static float angle = 0.0f;

SDL_AppResult SDL_AppIterate(void *appstate) {
    const uint64_t now = SDL_GetPerformanceCounter();
    const float dt =
        (float)(now - previous_counter) / (float)SDL_GetPerformanceFrequency();
    int width = 0;
    int height = 0;
    const float rotation_speed = 2.0f;
    const float camera_distance = 1.5f;
    const float point_size = 10.0f;
    angle += rotation_speed * dt;
    (void)appstate;
    previous_counter = now;

    const Mesh *mesh = &penger_mesh;

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

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    (void)appstate;

    if (event->type == SDL_EVENT_QUIT ||
        event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    (void)appstate;
    (void)result;

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
