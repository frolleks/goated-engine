#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>
#include <string.h>

#include "mesh.h"
#include "obj_to_mesh.h"
#include "renderer.h"
#include "texture_loader.h"

#include "penger.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
uint64_t previous_counter = 0;
static Mesh g_mesh = {0};
static ProjectedVertex *g_projected_vertices = NULL;
static Framebuffer g_framebuffer = {0};
static TextureImage *g_textures = NULL;
static size_t g_texture_count = 0;

static size_t mesh_storage_bytes(const Mesh *mesh) {
    if (!mesh) {
        return 0;
    }

    return (mesh->vertex_count * sizeof(*mesh->vertices)) + (mesh->triangle_count * sizeof(*mesh->triangles)) +
           (mesh->edge_count * sizeof(*mesh->edges)) + (mesh->material_count * sizeof(*mesh->materials));
}

static void destroy_textures(void) {
    if (!g_textures) {
        return;
    }

    for (size_t i = 0; i < g_texture_count; ++i) {
        texture_image_destroy(&g_textures[i]);
    }

    free(g_textures);
    g_textures = NULL;
    g_texture_count = 0;
}

static bool ensure_texture_capacity(size_t count) {
    if (count <= g_texture_count) {
        return true;
    }

    TextureImage *resized = (TextureImage *)realloc(g_textures, count * sizeof(*g_textures));
    if (!resized) {
        return false;
    }

    for (size_t i = g_texture_count; i < count; ++i) {
        resized[i] = (TextureImage){0};
    }

    g_textures = resized;
    g_texture_count = count;
    return true;
}

static bool load_mesh_textures(Mesh *mesh) {
    if (!mesh) {
        return false;
    }

    if (!ensure_texture_capacity(1)) {
        return false;
    }

    if (!texture_image_make_checker(&g_textures[0], 0xFFFF00FFu, 0xFF101010u)) {
        return false;
    }

    for (size_t i = 0; i < mesh->material_count; ++i) {
        Material *material = &mesh->materials[i];

        material->texture_index = 0;
        if (!material->diffuse_texture_path) {
            continue;
        }

        for (size_t loaded = 1; loaded < g_texture_count; ++loaded) {
            if (g_textures[loaded].path && strcmp(g_textures[loaded].path, material->diffuse_texture_path) == 0) {
                material->texture_index = (uint32_t)loaded;
                goto texture_ready;
            }
        }

        {
            const size_t next_index = g_texture_count;
            if (!ensure_texture_capacity(next_index + 1)) {
                return false;
            }

            if (!texture_image_load(material->diffuse_texture_path, &g_textures[next_index])) {
                g_texture_count = next_index;
                SDL_Log("Failed to load texture '%s', using fallback", material->diffuse_texture_path);
                material->texture_index = 0;
                continue;
            }

            material->texture_index = (uint32_t)next_index;
        }

    texture_ready:
        continue;
    }

    return true;
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

    g_projected_vertices = SDL_malloc(sizeof(*g_projected_vertices) * g_mesh.vertex_count);

    if (!g_projected_vertices) {
        SDL_Log("Failed to allocate projected vertex buffer");
        mesh_free(&g_mesh);
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("cube", 960, 540, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create the window and renderer: %s", SDL_GetError());
        mesh_free(&g_mesh);
        SDL_free(g_projected_vertices);
        g_projected_vertices = NULL;
        return SDL_APP_FAILURE;
    }

    if (!load_mesh_textures(&g_mesh)) {
        SDL_Log("Failed to load mesh textures");
        mesh_free(&g_mesh);
        SDL_free(g_projected_vertices);
        g_projected_vertices = NULL;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        renderer = NULL;
        window = NULL;
        destroy_textures();
        return SDL_APP_FAILURE;
    }

    previous_counter = SDL_GetPerformanceCounter();

    SDL_Log("vertices: %zu", g_mesh.vertex_count);
    SDL_Log("triangles: %zu", g_mesh.triangle_count);
    SDL_Log("materials: %zu", g_mesh.material_count);
    SDL_Log("loaded textures: %zu", g_texture_count);
    SDL_Log("mesh cpu storage: %.2f KB", (double)mesh_storage_bytes(&g_mesh) / 1024.0);
    SDL_Log("projected buffer: %.2f KB", (double)(g_mesh.vertex_count * sizeof(*g_projected_vertices)) / 1024.0);
    SDL_Log("renderer: %s", SDL_GetRendererName(renderer));

    return SDL_APP_CONTINUE;
}

static float angle = 0.0f;

SDL_AppResult SDL_AppIterate(void *appstate) {
    const uint64_t now = SDL_GetPerformanceCounter();
    const float dt = (float)(now - previous_counter) / (float)SDL_GetPerformanceFrequency();

    const float rotation_speed = 2.0f;
    const float camera_distance = 1.5f;

    angle += rotation_speed * dt;
    (void)appstate;
    previous_counter = now;

    if (!draw_mesh_textured(renderer, &g_framebuffer, &g_mesh, g_textures, g_texture_count, g_projected_vertices, angle,
                            camera_distance)) {
        SDL_Log("Couldn't draw the textured mesh: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    (void)appstate;

    if (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    (void)appstate;
    (void)result;

    mesh_free(&g_mesh);
    destroy_textures();
    framebuffer_destroy(&g_framebuffer);

    SDL_free(g_projected_vertices);
    g_projected_vertices = NULL;

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
