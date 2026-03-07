#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

SDL_FPoint normalize_coords(float x, float y, int width, int height)
{
    float scale = SDL_min(width, height) * 0.5f;

    const float half_width = width * 0.5f;
    const float half_height = height * 0.5f;

    return (SDL_FPoint){
        half_width + (x * scale),
        half_height - (y * scale),
    };
}

/* Called once when SDL starts: set up the video system, window, and renderer. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
    (void)appstate;
    (void)argc;
    (void)argv;

    SDL_SetAppMetadata("Hello Triangle", "1.0", "com.frolleks.sdl-learning");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Hello, Triangle!", 960, 540, SDL_WINDOW_RESIZABLE, &window, &renderer))
    {
        SDL_Log("Couldn't create the window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

/* Called every frame: build the triangle, clear the screen, draw, then present. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    int width = 0;
    int height = 0;
    const SDL_FColor black = {0.0f, 0.0f, 0.0f, 1.0f};
    SDL_Vertex triangle[3];

    (void)appstate;

    if (!SDL_GetRenderOutputSize(renderer, &width, &height))
    {
        SDL_Log("Couldn't get renderer size: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    /* Vertices are in centered normalized space: (0, 0) is screen center. */
    triangle[0] = (SDL_Vertex){normalize_coords(0.0f, 0.0f, width, height), black, {0.0f, 0.0f}};
    triangle[1] = (SDL_Vertex){normalize_coords(0.25f, -0.5f, width, height), black, {0.0f, 0.0f}};
    triangle[2] = (SDL_Vertex){normalize_coords(-0.25f, -0.5f, width, height), black, {0.0f, 0.0f}};

    if (!SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE))
    {
        SDL_Log("Couldn't set the clear color: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_RenderClear(renderer))
    {
        SDL_Log("Couldn't clear the frame: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_RenderGeometry(renderer, NULL, triangle, 3, NULL, 0))
    {
        SDL_Log("Couldn't draw the triangle: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_RenderPresent(renderer))
    {
        SDL_Log("Couldn't present the frame: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

/* Called for incoming OS events. We only care about closing the window for now. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    (void)appstate;

    if (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
    {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

/* Called once on exit: release SDL objects in the opposite order they were created. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    (void)appstate;
    (void)result;

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
