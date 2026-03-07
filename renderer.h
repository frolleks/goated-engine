#ifndef RENDERER_H
#define RENDERER_H

#include <SDL3/SDL.h>
#include <stdbool.h>

#include "math3d.h"
#include "mesh.h"
#include "normalize_coords.h"

bool render_triangle_outline(SDL_Renderer *renderer, SDL_FPoint a, SDL_FPoint b,
                             SDL_FPoint c);
SDL_FPoint project_to_2d(float x, float y, float z);
bool draw_mesh_wireframe(SDL_Renderer *renderer, const Mesh *mesh, float angle,
                         float camera_distance, int width, int height);

#endif