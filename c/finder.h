#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "color.h"

typedef struct edge {
    uint32_t x, y;
    color color;
} edge;

typedef struct finder {
    bool  (*add)(struct finder *, edge);
    bool  (*remove)(struct finder *, edge);
    float (*nearest)(const struct finder *, color, edge *);
    void  (*free)(const struct finder *);
} finder;

static inline bool
finder_add(finder *f, edge e)
{
    return f->add(f, e);
}

static inline bool
finder_remove(finder *f, edge e)
{
    return f->remove(f, e);
}

static inline float
finder_nearest(finder *f, color c, edge *e)
{
    return f->nearest(f, c, e);
}

static inline void
finder_free(const finder *f)
{
    f->free(f);
}
