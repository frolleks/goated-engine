#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "mesh.h"
#include "obj_to_mesh.h"
#include "renderer.h"

#include "penger.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
uint64_t previous_counter = 0;
static Mesh g_mesh = {0};
static SDL_FPoint *g_projected_vertices = NULL;

static size_t mesh_wireframe_segment_count(const Mesh *mesh) {
    if (!mesh) {
        return 0;
    }

    if (mesh->edges && mesh->edge_count > 0) {
        return mesh->edge_count;
    }

    return mesh->triangle_count * 3;
}

static size_t mesh_storage_bytes(const Mesh *mesh) {
    if (!mesh) {
        return 0;
    }

    return (mesh->vertex_count * sizeof(*mesh->vertices)) +
           (mesh->triangle_count * sizeof(*mesh->triangles)) +
           (mesh->edge_count * sizeof(*mesh->edges));
}

static void prefer_low_memory_renderer(const Mesh *mesh) {
    static const size_t low_memory_wireframe_threshold = 50000;
    const char *configured_driver = SDL_GetHint(SDL_HINT_RENDER_DRIVER);
    const size_t segment_count = mesh_wireframe_segment_count(mesh);

    if (configured_driver && configured_driver[0] != '\0') {
        return;
    }

    if (segment_count < low_memory_wireframe_threshold) {
        return;
    }

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    SDL_Log("Using SDL software renderer for %zu wireframe edges to reduce RAM "
            "usage",
            segment_count);
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

    fastObjMesh *obj_mesh = fast_obj_read("assets/file.obj");

    if (!obj_mesh) {
        SDL_Log("Failed to load OBJ");
        return SDL_APP_FAILURE;
    }

    if (!convert_fastobj_to_mesh(obj_mesh, &g_mesh)) {
        SDL_Log("Failed to convert OBJ to Mesh");
        fast_obj_destroy(obj_mesh);
        return SDL_APP_FAILURE;
    }

    fast_obj_destroy(obj_mesh);

    g_projected_vertices =
        SDL_malloc(sizeof(*g_projected_vertices) * g_mesh.vertex_count);

    if (!g_projected_vertices) {
        SDL_Log("Failed to allocate projected vertex buffer");
        mesh_free(&g_mesh);
        return SDL_APP_FAILURE;
    }

    prefer_low_memory_renderer(&g_mesh);

    if (!SDL_CreateWindowAndRenderer("cube", 960, 540, SDL_WINDOW_RESIZABLE,
                                     &window, &renderer)) {
        SDL_Log("Couldn't create the window and renderer: %s", SDL_GetError());
        mesh_free(&g_mesh);
        SDL_free(g_projected_vertices);
        g_projected_vertices = NULL;
        return SDL_APP_FAILURE;
    }

    previous_counter = SDL_GetPerformanceCounter();

    SDL_Log("vertices: %zu", g_mesh.vertex_count);
    SDL_Log("triangles: %zu", g_mesh.triangle_count);
    SDL_Log("wireframe edges: %zu", g_mesh.edge_count);
    SDL_Log("mesh cpu storage: %.2f KB",
            (double)mesh_storage_bytes(&g_mesh) / 1024.0);
    SDL_Log("projected buffer: %.2f KB",
            (double)(g_mesh.vertex_count * sizeof(*g_projected_vertices)) /
                1024.0);
    SDL_Log("renderer: %s", SDL_GetRendererName(renderer));

    return SDL_APP_CONTINUE;
}

static float angle = 0.0f;

SDL_AppResult SDL_AppIterate(void *appstate) {
    const uint64_t now = SDL_GetPerformanceCounter();
    const float dt =
        (float)(now - previous_counter) / (float)SDL_GetPerformanceFrequency();

    const float rotation_speed = 2.0f;
    const float camera_distance = 2.0f;

    angle += rotation_speed * dt;
    (void)appstate;
    previous_counter = now;

    if (!draw_mesh_wireframe(renderer, &g_mesh, g_projected_vertices, angle,
                             camera_distance)) {
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

    mesh_free(&g_mesh);

    SDL_free(g_projected_vertices);
    g_projected_vertices = NULL;

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
