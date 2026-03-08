#ifndef RENDERER_H
#define RENDERER_H

#include <SDL3/SDL.h>
#include <stdbool.h>

#include "math3d.h"
#include "mesh.h"
#include "normalize_coords.h"
#include "texture_loader.h"

typedef struct {
    float screen_x;
    float screen_y;
    float camera_x;
    float camera_y;
    float camera_z;
    float inv_z;
    float u_over_z;
    float v_over_z;
} ProjectedVertex;

typedef struct {
    SDL_Texture *texture;
    uint32_t *color_buffer;
    float *depth_buffer;
    int width;
    int height;
} Framebuffer;

bool render_triangle_outline(SDL_Renderer *renderer, SDL_FPoint a, SDL_FPoint b, SDL_FPoint c);
SDL_FPoint project_to_2d(float x, float y, float z);
void framebuffer_destroy(Framebuffer *framebuffer);
bool draw_mesh_textured(SDL_Renderer *renderer, Framebuffer *framebuffer, const Mesh *mesh,
                        const TextureImage *textures, size_t texture_count, ProjectedVertex *projected_vertices,
                        float angle, float camera_distance);

#endif
