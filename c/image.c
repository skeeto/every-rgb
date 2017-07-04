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
    uint8_t *buffer = malloc(im->width * im->height * 3);
    fprintf(out, "P6\n%d %d\n255\n", im->width, im->height);
    float inv = 1.0f / gamma;
    for (uint32_t y = 0; y < im->height; y++) {
        for (uint32_t x = 0; x < im->width; x++) {
            color color = image_get(im, x, y);
            uint8_t *p = buffer + (y * im->width + x) * 3;
            p[0] = powf(color.p.r, inv) * 255;
            p[1] = powf(color.p.g, inv) * 255;
            p[2] = powf(color.p.b, inv) * 255;
        }
    }
    fwrite(buffer, im->width * im->height, 3, out);
    fflush(out);
    free(buffer);
}

void
image_free(const image *image)
{
    free((void *)image);
}
