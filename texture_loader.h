#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    char *path;
    uint32_t *pixels;
    int width;
    int height;
} TextureImage;

bool texture_image_load(const char *path, TextureImage *texture);
bool texture_image_make_checker(TextureImage *texture, uint32_t a, uint32_t b);
void texture_image_destroy(TextureImage *texture);

#endif
