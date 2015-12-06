#pragma once

#include <stdint.h>
#include "color.h"

typedef struct image {
    uint32_t width;
    uint32_t height;
    color pixels[];
} image;

image *image_create(uint32_t width, uint32_t height);
void   image_free(const image *image);
void   image_save(image *im, float gamma, FILE *out);

static inline color
image_get(const image *im, uint32_t x, uint32_t y)
{
    if (x < im->width && y < im->height)
        return im->pixels[y * im->width + x];
    else
        return COLOR(0, 0, 0, 1);
}

static inline void
image_set(image *im, uint32_t x, uint32_t y, color color)
{
    im->pixels[y * im->width + x] = color;
}
