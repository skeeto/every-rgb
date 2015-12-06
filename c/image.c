#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "image.h"

image *
image_create(uint32_t width, uint32_t height)
{
    image *image;
    size_t size = sizeof(*image) + width * height * sizeof(image->pixels[0]);
    image = calloc(size, 1);
    image->width = width;
    image->height = height;
    return image;
}
void
image_save(image *im, float gamma, FILE *out)
{
    fprintf(out, "P6\n%d %d\n255\n", im->width, im->height);
    flockfile(out);
    float inv = 1.0f / gamma;
    for (uint32_t y = 0; y < im->height; y++) {
        for (uint32_t x = 0; x < im->width; x++) {
            color color = image_get(im, x, y);
            fputc_unlocked(powf(color.r, inv) * 255, out);
            fputc_unlocked(powf(color.g, inv) * 255, out);
            fputc_unlocked(powf(color.b, inv) * 255, out);
        }
    }
    funlockfile(out);
    fflush(stdout);
}

void
image_free(const image *image)
{
    free((void *)image);
}
