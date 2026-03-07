#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>

#include "normalize_coords.h"
#include "object.h"
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
    SDL_FPoint projected_vertices[SDL_arraysize(object_vertices)];
    const float rotation_speed = 2.0f;
    const float camera_distance = 1.5f;
    const float point_size = 10.0f;
    angle += rotation_speed * dt;
    (void)appstate;
    previous_counter = now;

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

    for (int i = 0; i < (int)SDL_arraysize(object_vertices); ++i) {
        struct VerticesStruct rotated_coords =
            rotate_xz(object_vertices[i].x, object_vertices[i].y,
                      object_vertices[i].z, angle);

        rotated_coords.z += camera_distance;

        SDL_FPoint first_vs =
            project_to_2d(rotated_coords.x, rotated_coords.y, rotated_coords.z);
        SDL_FPoint rect_pos =
            normalize_coords(first_vs.x, first_vs.y, width, height);
        projected_vertices[i] = rect_pos;
        const SDL_FRect rect = {
            rect_pos.x - (point_size * 0.5f),
            rect_pos.y - (point_size * 0.5f),
            point_size,
            point_size,
        };

        // if (!SDL_RenderFillRect(renderer, &rect)) {
        //     SDL_Log("Couldn't draw the rectangle: %s", SDL_GetError());
        //     return SDL_APP_FAILURE;
        // }
    }

    for (int i = 0; i < (int)SDL_arraysize(object_faces); ++i) {
        const struct Triangle triangle = object_faces[i];
        const SDL_FPoint a = projected_vertices[triangle.v1];
        const SDL_FPoint b = projected_vertices[triangle.v2];
        const SDL_FPoint c = projected_vertices[triangle.v3];

        if (!render_triangle_outline(renderer, a, b, c)) {
            SDL_Log("Couldn't draw the triangle outline: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }
    }

    if (!SDL_RenderPresent(renderer)) {
        SDL_Log("Couldn't present the frame: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

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
