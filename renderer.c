#include "renderer.h"

#include <stdlib.h>
#include <string.h>

static float edge_function(float ax, float ay, float bx, float by, float px, float py) {
    return ((px - ax) * (by - ay)) - ((py - ay) * (bx - ax));
}

static int clamp_int(int value, int min_value, int max_value) {
    if (value < min_value) {
        return min_value;
    }

    if (value > max_value) {
        return max_value;
    }

    return value;
}

static void clear_framebuffer(Framebuffer *framebuffer, uint32_t color) {
    const size_t pixel_count = (size_t)framebuffer->width * (size_t)framebuffer->height;

    for (size_t i = 0; i < pixel_count; ++i) {
        framebuffer->color_buffer[i] = color;
        framebuffer->depth_buffer[i] = 0.0f;
    }
}

static bool ensure_framebuffer(SDL_Renderer *renderer, Framebuffer *framebuffer) {
    int width = 0;
    int height = 0;

    if (!SDL_GetRenderOutputSize(renderer, &width, &height)) {
        SDL_Log("Couldn't get renderer size: %s", SDL_GetError());
        return false;
    }

    if (framebuffer->width == width && framebuffer->height == height && framebuffer->texture &&
        framebuffer->color_buffer && framebuffer->depth_buffer) {
        return true;
    }

    framebuffer_destroy(framebuffer);

    framebuffer->texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!framebuffer->texture) {
        SDL_Log("Couldn't create framebuffer texture: %s", SDL_GetError());
        framebuffer_destroy(framebuffer);
        return false;
    }

    framebuffer->color_buffer = (uint32_t *)malloc((size_t)width * (size_t)height * sizeof(uint32_t));
    framebuffer->depth_buffer = (float *)malloc((size_t)width * (size_t)height * sizeof(float));
    if (!framebuffer->color_buffer || !framebuffer->depth_buffer) {
        SDL_Log("Couldn't allocate framebuffer memory");
        framebuffer_destroy(framebuffer);
        return false;
    }

    framebuffer->width = width;
    framebuffer->height = height;
    return true;
}

static const TextureImage *select_triangle_texture(const Mesh *mesh, const TextureImage *textures, size_t texture_count,
                                                   Triangle triangle) {
    if (!textures || texture_count == 0) {
        return NULL;
    }

    if (triangle.material_index < mesh->material_count) {
        const Material material = mesh->materials[triangle.material_index];
        if (material.texture_index < texture_count) {
            return &textures[material.texture_index];
        }
    }

    return &textures[0];
}

static uint32_t blend_rgba32(uint32_t dst, uint32_t src) {
    const uint32_t src_a = (src >> 24) & 0xFFu;
    if (src_a >= 255u) {
        return src;
    }

    const uint32_t inv_a = 255u - src_a;
    const uint32_t src_r = src & 0xFFu;
    const uint32_t src_g = (src >> 8) & 0xFFu;
    const uint32_t src_b = (src >> 16) & 0xFFu;
    const uint32_t dst_r = dst & 0xFFu;
    const uint32_t dst_g = (dst >> 8) & 0xFFu;
    const uint32_t dst_b = (dst >> 16) & 0xFFu;

    const uint32_t out_r = ((src_r * src_a) + (dst_r * inv_a)) / 255u;
    const uint32_t out_g = ((src_g * src_a) + (dst_g * inv_a)) / 255u;
    const uint32_t out_b = ((src_b * src_a) + (dst_b * inv_a)) / 255u;

    return 0xFF000000u | (out_b << 16) | (out_g << 8) | out_r;
}

static void rasterize_triangle(Framebuffer *framebuffer, const TextureImage *texture, ProjectedVertex a,
                               ProjectedVertex b, ProjectedVertex c, bool blend_alpha) {
    if (!texture || !texture->pixels || texture->width <= 0 || texture->height <= 0) {
        return;
    }

    const float area = edge_function(a.screen_x, a.screen_y, b.screen_x, b.screen_y, c.screen_x, c.screen_y);
    if (SDL_fabsf(area) < 0.0001f) {
        return;
    }

    int min_x = (int)SDL_floorf(SDL_min(a.screen_x, SDL_min(b.screen_x, c.screen_x)));
    int max_x = (int)SDL_ceilf(SDL_max(a.screen_x, SDL_max(b.screen_x, c.screen_x)));
    int min_y = (int)SDL_floorf(SDL_min(a.screen_y, SDL_min(b.screen_y, c.screen_y)));
    int max_y = (int)SDL_ceilf(SDL_max(a.screen_y, SDL_max(b.screen_y, c.screen_y)));

    min_x = clamp_int(min_x, 0, framebuffer->width - 1);
    max_x = clamp_int(max_x, 0, framebuffer->width - 1);
    min_y = clamp_int(min_y, 0, framebuffer->height - 1);
    max_y = clamp_int(max_y, 0, framebuffer->height - 1);

    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            const float px = (float)x + 0.5f;
            const float py = (float)y + 0.5f;

            const float w0 = edge_function(b.screen_x, b.screen_y, c.screen_x, c.screen_y, px, py);
            const float w1 = edge_function(c.screen_x, c.screen_y, a.screen_x, a.screen_y, px, py);
            const float w2 = edge_function(a.screen_x, a.screen_y, b.screen_x, b.screen_y, px, py);

            if ((w0 < 0.0f || w1 < 0.0f || w2 < 0.0f) && (w0 > 0.0f || w1 > 0.0f || w2 > 0.0f)) {
                continue;
            }

            const float alpha = w0 / area;
            const float beta = w1 / area;
            const float gamma = w2 / area;

            const float inv_z = (alpha * a.inv_z) + (beta * b.inv_z) + (gamma * c.inv_z);
            if (inv_z <= 0.0f) {
                continue;
            }

            const size_t pixel_index = (size_t)y * (size_t)framebuffer->width + (size_t)x;
            if (inv_z <= framebuffer->depth_buffer[pixel_index]) {
                continue;
            }

            const float u_over_z = (alpha * a.u_over_z) + (beta * b.u_over_z) + (gamma * c.u_over_z);
            const float v_over_z = (alpha * a.v_over_z) + (beta * b.v_over_z) + (gamma * c.v_over_z);

            const float u = u_over_z / inv_z;
            const float v = v_over_z / inv_z;

            int tex_x = (int)(u * (float)(texture->width - 1));
            int tex_y = (int)(v * (float)(texture->height - 1));

            tex_x = clamp_int(tex_x, 0, texture->width - 1);
            tex_y = clamp_int(tex_y, 0, texture->height - 1);

            const uint32_t sample = texture->pixels[(size_t)tex_y * (size_t)texture->width + (size_t)tex_x];
            const uint32_t sample_alpha = (sample >> 24) & 0xFFu;
            if (sample_alpha == 0) {
                continue;
            }

            if (blend_alpha) {
                framebuffer->color_buffer[pixel_index] =
                    blend_rgba32(framebuffer->color_buffer[pixel_index], sample);
            } else {
                framebuffer->depth_buffer[pixel_index] = inv_z;
                framebuffer->color_buffer[pixel_index] = sample;
            }
        }
    }
}

void framebuffer_destroy(Framebuffer *framebuffer) {
    if (!framebuffer) {
        return;
    }

    if (framebuffer->texture) {
        SDL_DestroyTexture(framebuffer->texture);
    }

    free(framebuffer->color_buffer);
    free(framebuffer->depth_buffer);

    memset(framebuffer, 0, sizeof(*framebuffer));
}

SDL_FPoint project_to_2d(float x, float y, float z) { return (SDL_FPoint){x / z, y / z}; }

bool render_triangle_outline(SDL_Renderer *renderer, SDL_FPoint a, SDL_FPoint b, SDL_FPoint c) {
    return SDL_RenderLine(renderer, a.x, a.y, b.x, b.y) && SDL_RenderLine(renderer, b.x, b.y, c.x, c.y) &&
           SDL_RenderLine(renderer, c.x, c.y, a.x, a.y);
}

bool draw_mesh_textured(SDL_Renderer *renderer, Framebuffer *framebuffer, const Mesh *mesh,
                        const TextureImage *textures, size_t texture_count, ProjectedVertex *projected_vertices,
                        float angle, float camera_distance) {
    if (!renderer || !framebuffer || !mesh || !projected_vertices) {
        return false;
    }

    if (!ensure_framebuffer(renderer, framebuffer)) {
        return false;
    }

    clear_framebuffer(framebuffer, 0xFF101018u);

    for (size_t i = 0; i < mesh->vertex_count; ++i) {
        const Vertex rotated = rotate_y(mesh->vertices[i], angle);
        ProjectedVertex *projected = &projected_vertices[i];
        const float z = rotated.z + camera_distance;

        if (z <= 0.01f) {
            memset(projected, 0, sizeof(*projected));
            continue;
        }

        const SDL_FPoint projected_xy = project_to_2d(rotated.x, rotated.y, z);
        const SDL_FPoint normalized =
            normalize_coords(projected_xy.x, projected_xy.y, framebuffer->width, framebuffer->height);

        projected->screen_x = normalized.x;
        projected->screen_y = normalized.y;
        projected->camera_x = rotated.x;
        projected->camera_y = rotated.y;
        projected->camera_z = z;
        projected->inv_z = 1.0f / z;
        projected->u_over_z = rotated.u * projected->inv_z;
        projected->v_over_z = rotated.v * projected->inv_z;
    }

    for (int pass = 0; pass < 2; ++pass) {
        const bool transparent_pass = (pass == 1);

        for (size_t i = 0; i < mesh->triangle_count; ++i) {
            const Triangle triangle = mesh->triangles[i];
            const ProjectedVertex a = projected_vertices[triangle.v1];
            const ProjectedVertex b = projected_vertices[triangle.v2];
            const ProjectedVertex c = projected_vertices[triangle.v3];
            const TextureImage *texture =
                select_triangle_texture(mesh, textures, texture_count, triangle);
            const bool is_transparent = texture && texture->has_transparency;

            if (is_transparent != transparent_pass) {
                continue;
            }

            if (a.inv_z <= 0.0f || b.inv_z <= 0.0f || c.inv_z <= 0.0f) {
                continue;
            }

            if (!is_transparent) {
                const float ab_x = b.camera_x - a.camera_x;
                const float ab_y = b.camera_y - a.camera_y;
                const float ab_z = b.camera_z - a.camera_z;
                const float ac_x = c.camera_x - a.camera_x;
                const float ac_y = c.camera_y - a.camera_y;
                const float ac_z = c.camera_z - a.camera_z;

                const float normal_x = (ab_y * ac_z) - (ab_z * ac_y);
                const float normal_y = (ab_z * ac_x) - (ab_x * ac_z);
                const float normal_z = (ab_x * ac_y) - (ab_y * ac_x);
                const float facing =
                    (normal_x * a.camera_x) + (normal_y * a.camera_y) + (normal_z * a.camera_z);

                if (facing >= 0.0f) {
                    continue;
                }
            }

            rasterize_triangle(framebuffer, texture, a, b, c, is_transparent);
        }
    }

    if (!SDL_UpdateTexture(framebuffer->texture, NULL, framebuffer->color_buffer,
                           framebuffer->width * (int)sizeof(uint32_t))) {
        SDL_Log("Couldn't upload framebuffer: %s", SDL_GetError());
        return false;
    }

    if (!SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE)) {
        SDL_Log("Couldn't set clear color: %s", SDL_GetError());
        return false;
    }

    if (!SDL_RenderClear(renderer)) {
        SDL_Log("Couldn't clear renderer: %s", SDL_GetError());
        return false;
    }

    if (!SDL_RenderTexture(renderer, framebuffer->texture, NULL, NULL)) {
        SDL_Log("Couldn't render framebuffer texture: %s", SDL_GetError());
        return false;
    }

    if (!SDL_RenderPresent(renderer)) {
        SDL_Log("Couldn't present the frame: %s", SDL_GetError());
        return false;
    }

    return true;
}
