#pragma once

#include <stdint.h>
#include "color.h"

typedef struct colorset {
    int depth;
    size_t count;
    color colors[];
} colorset;

colorset *colorset_create(int depth, float gamma);
void      colorset_free(const colorset *);
color     colorset_pop(colorset *);
void      colorset_shuffle(colorset *, uint64_t *);
void      colorset_sort(colorset *);
