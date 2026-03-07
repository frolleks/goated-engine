#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>

#include "normalize_coords.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static uint64_t previous_counter = 0;

struct VerticesStruct {
    float x;
    float y;
    float z;
};

struct Edge {
    int from;
    int to;
};

static struct VerticesStruct vertices[] = {
    {0.25f, 0.25f, 0.25f},    {-0.25f, 0.25f, 0.25f},
    {-0.25f, -0.25f, 0.25f},  {0.25f, -0.25f, 0.25f},

    {0.25f, 0.25f, -0.25f},   {-0.25f, 0.25f, -0.25f},
    {-0.25f, -0.25f, -0.25f}, {0.25f, -0.25f, -0.25f},
};

static struct Edge edges[] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6},
    {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7},
};

SDL_FPoint project_to_2d(float x, float y, float z) {
    return (SDL_FPoint){x / z, y / z};
}

struct VerticesStruct rotate_xz(float x, float y, float z, float angle) {
    const float c = cosf(angle);
    const float s = sinf(angle);

    return (struct VerticesStruct){
        (x * c) - (z * s),
        y,
        (x * s) + (z * c),
    };
}

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
    SDL_FPoint projected_vertices[SDL_arraysize(vertices)];
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

    for (int i = 0; i < (int)SDL_arraysize(vertices); ++i) {
        struct VerticesStruct rotated_coords =
            rotate_xz(vertices[i].x, vertices[i].y, vertices[i].z, angle);

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

        if (!SDL_RenderFillRect(renderer, &rect)) {
            SDL_Log("Couldn't draw the rectangle: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }
    }

    for (int i = 0; i < (int)SDL_arraysize(edges); ++i) {
        const struct Edge edge = edges[i];
        const SDL_FPoint a = projected_vertices[edge.from];
        const SDL_FPoint b = projected_vertices[edge.to];

        if (!SDL_RenderLine(renderer, a.x, a.y, b.x, b.y)) {
            SDL_Log("Couldn't draw the line: %s", SDL_GetError());
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
