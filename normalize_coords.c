#include "normalize_coords.h"

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