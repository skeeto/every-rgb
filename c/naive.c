#include <stdlib.h>
#include <assert.h>
#include "naive.h"

static bool
method_add(struct finder *f, edge e)
{
    return naive_add((naive *)f, e);
}

static bool
method_remove(struct finder *f, edge e)
{
    return naive_remove((naive *)f, e);
}

static float
method_nearest(const struct finder *f, color c, edge *e)
{
    return naive_nearest((const naive *)f, c, e);
}

static void
method_free(const struct finder *f)
{
    naive_free((naive *)f);
}

finder *
naive_create(void)
{
    naive *naive = malloc(sizeof(*naive));
    naive->max = 4096;
    naive->count = 0;
    naive->edges = malloc(naive->max * sizeof(naive->edges[0]));
    finder *f = &naive->finder;
    f->add = method_add;
    f->remove = method_remove;
    f->nearest = method_nearest;
    f->free = method_free;
    return f;
}

void
naive_free(const naive *naive)
{
    free(naive->edges);
    free((void *)naive);
}

bool
naive_add(naive *naive, edge edge)
{
    if (naive->count == naive->max) {
        naive->max *= 2;
        naive->edges =
            realloc(naive->edges, naive->max * sizeof(naive->edges[0]));
    }
    naive->edges[naive->count++] = edge;
    return true;
}

float
naive_nearest(const naive *naive, color target, edge *edge)
{
    *edge = naive->edges[0];
    float best2 = color_dist2(edge->color, target);
    for (size_t i = 1; i < naive->count; i++) {
        float dist2 = color_dist2(naive->edges[i].color, target);
        if (dist2 < best2) {
            best2 = dist2;
            *edge = naive->edges[i];
        }
    }
    return sqrtf(best2);
}

bool
naive_remove(naive *naive, edge e)
{
    for (size_t i = 0; i < naive->count; i++) {
        if ((naive->edges[i].x == e.x) && (naive->edges[i].y == e.y)) {
            naive->edges[i] = naive->edges[--naive->count];
            return true;
        }
    }
    return false;
}
