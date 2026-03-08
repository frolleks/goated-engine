#include "texture_loader.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <stdlib.h>
#include <string.h>

static char *copy_string(const char *src) {
    if (!src) {
        return NULL;
    }

    const size_t len = strlen(src);
    char *copy = (char *)malloc(len + 1);
    if (!copy) {
        return NULL;
    }

    memcpy(copy, src, len + 1);
    return copy;
}

bool texture_image_load(const char *path, TextureImage *texture) {
    if (!path || !texture) {
        return false;
    }

    memset(texture, 0, sizeof(*texture));

    SDL_Surface *loaded = IMG_Load(path);
    if (!loaded) {
        return false;
    }

    SDL_Surface *converted = SDL_ConvertSurface(loaded, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(loaded);
    if (!converted) {
        return false;
    }

    const size_t row_bytes = (size_t)converted->w * sizeof(uint32_t);
    const size_t pixel_bytes = row_bytes * (size_t)converted->h;
    uint32_t *pixels = (uint32_t *)malloc(pixel_bytes);
    if (!pixels) {
        SDL_DestroySurface(converted);
        return false;
    }

    for (int y = 0; y < converted->h; ++y) {
        const uint8_t *src_row = (const uint8_t *)converted->pixels + ((size_t)y * converted->pitch);
        uint8_t *dst_row = (uint8_t *)pixels + ((size_t)y * row_bytes);
        memcpy(dst_row, src_row, row_bytes);
    }

    texture->path = copy_string(path);
    if (!texture->path) {
        free(pixels);
        SDL_DestroySurface(converted);
        return false;
    }

    texture->pixels = pixels;
    texture->width = converted->w;
    texture->height = converted->h;
    SDL_DestroySurface(converted);
    return true;
}

bool texture_image_make_checker(TextureImage *texture, uint32_t a, uint32_t b) {
    if (!texture) {
        return false;
    }

    memset(texture, 0, sizeof(*texture));

    texture->width = 2;
    texture->height = 2;
    texture->pixels = (uint32_t *)malloc((size_t)texture->width * texture->height * 4);
    if (!texture->pixels) {
        return false;
    }

    texture->pixels[0] = a;
    texture->pixels[1] = b;
    texture->pixels[2] = b;
    texture->pixels[3] = a;
    return true;
}

void texture_image_destroy(TextureImage *texture) {
    if (!texture) {
        return;
    }

    free(texture->path);
    free(texture->pixels);

    texture->path = NULL;
    texture->pixels = NULL;
    texture->width = 0;
    texture->height = 0;
}
