#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "mesh.h"
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

    if (!draw_mesh_wireframe(renderer, mesh, angle, camera_distance, width,
                             height)) {
        SDL_Log("Couldn't draw the wireframe: %s", SDL_GetError());
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
