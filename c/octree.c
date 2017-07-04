#include <stdlib.h>
#include <float.h>
#include <assert.h>
#include "octree.h"
#include "color.h"

static octree *
octree_init(octree *octree, color bound[2])
{
    octree->nodes = NULL;
    octree->count = 0;
    octree->bound[0] = bound[0];
    octree->bound[1] = bound[1];
    return octree;
}

void octree_free(const octree *octree)
{
    if (octree->nodes)
        for (size_t i = 0; i < 8; i++)
            octree_free(octree->nodes + i);
    free(octree->nodes);
}

static bool
octree_in_bounds(const octree *octree, color color)
{
    return
        color.p.r >= octree->bound[0].p.r &&
        color.p.r <  octree->bound[1].p.r &&
        color.p.g >= octree->bound[0].p.g &&
        color.p.g <  octree->bound[1].p.g &&
        color.p.b >= octree->bound[0].p.b &&
        color.p.b <  octree->bound[1].p.b;
}

static bool
octree_in_radius(const octree *octree, color color, float radius)
{
    return
        color.p.r >= octree->bound[0].p.r - radius &&
        color.p.r <  octree->bound[1].p.r + radius &&
        color.p.g >= octree->bound[0].p.g - radius &&
        color.p.g <  octree->bound[1].p.g + radius &&
        color.p.b >= octree->bound[0].p.b - radius &&
        color.p.b <  octree->bound[1].p.b + radius;
}



static void
octree_split(octree *octree)
{
    assert(!octree->nodes);
    octree->nodes = malloc(sizeof(octree->nodes[0]) * 8);
    color c0 = octree->bound[0];
    color c1 = octree->bound[1];
    int i = 0;
    for (int r = 0; r < 2; r++) {
        float hr = (c1.p.r - c0.p.r) / 2.0f;
        for (int g = 0; g < 2; g++) {
            float hg = (c1.p.g - c0.p.g) / 2.0f;
            for (int b = 0; b < 2; b++) {
                float hb = (c1.p.b - c0.p.b) / 2.0f;
                color bounds[2];
                bounds[0].p.r = c0.p.r + hr * r;
                bounds[0].p.g = c0.p.g + hg * g;
                bounds[0].p.b = c0.p.b + hb * b;
                bounds[1].p.r = bounds[0].p.r + hr;
                bounds[1].p.g = bounds[0].p.g + hg;
                bounds[1].p.b = bounds[0].p.b + hb;
                octree_init(octree->nodes + i++, bounds);
            }
        }
    }
    /* Move colors to children. */
    for (size_t i = 0; i < octree->count; i++) {
        for (int n = 0; n < 8; n++)
            if (octree_add(octree->nodes + n, octree->edges[i]))
                break;
    }
}

static void
octree_coalesce(octree *octree)
{
    assert(octree->nodes);
    assert(octree->count <= OCTREE_THRESHOLD);
    size_t count = 0;
    for (int i = 0; i < 8; i++) {
        for (size_t c = 0; c < octree->nodes[i].count; c++)
            octree->edges[count++] = octree->nodes[i].edges[c];
        octree_free(octree->nodes + i);
    }
    assert(count == octree->count);
    free(octree->nodes);
    octree->nodes = NULL;
}

bool
octree_add(octree *octree, edge edge)
{
    if (!octree_in_bounds(octree, edge.color))
        return false;
    if (!octree->nodes && octree->count == OCTREE_THRESHOLD)
        octree_split(octree);
    if (!octree->nodes)
        octree->edges[octree->count++] = edge;
    else
        for (size_t i = 0; i < 8; i++)
            if (octree_add(octree->nodes + i, edge)) {
                octree->count++;
                break;
            }
    return true;
}

static float
octree_leaf_closest(const octree *octree, color target, edge *out)
{
    assert(!octree->nodes);
    assert(octree->count > 0);
    *out = octree->edges[0];
    float best2 = color_dist2(target, out->color);
    for (size_t i = 1; i < octree->count; i++) {
        float dist2 = color_dist2(target, octree->edges[i].color);
        if (dist2 < best2) {
            best2 = dist2;
            *out = octree->edges[i];
        }
    }
    assert(isfinite(best2));
    return best2;
}

static float
octree_find(const octree *octree, color target, edge *out)
{
    if (octree->count == 0 || !octree_in_bounds(octree, target))
        return INFINITY;
    if (!octree->nodes) {
        return octree_leaf_closest(octree, target, out);
    } else {
        float best2 = octree_find(octree->nodes + 0, target, out);
        for (size_t i = 1; i < 8; i++) {
            edge e;
            float dist2 = octree_find(octree->nodes + i, target, &e);
            if (dist2 < best2) {
                best2 = dist2;
                *out = e;
            }
        }
        return best2;
    }
}

static float
octree_nearest_radius(const octree *o, color target, edge *out, float r)
{
    if (o->count == 0 || !octree_in_radius(o, target, r))
        return INFINITY;
    if (!o->nodes) {
        return octree_leaf_closest(o, target, out);
    } else {
        float best2 = octree_nearest_radius(o->nodes + 0, target, out, r);
        for (size_t i = 1; i < 8; i++) {
            edge e;
            float dist2 = octree_nearest_radius(o->nodes + i, target, &e, r);
            if (dist2 < best2) {
                best2 = dist2;
                *out = e;
            }
        }
        return best2;
    }
}

float
octree_nearest(const octree *octree, color target, edge *out)
{
    float worst = sqrtf(octree_find(octree, target, out));
    if (isfinite(worst))
        return sqrtf(octree_nearest_radius(octree, target, out, worst));
    else {
        float radius = 0.1;
        do {
            worst = sqrtf(octree_nearest_radius(octree, target, out, radius));
            radius *= 2;
            assert(isfinite(radius));
        } while (!isfinite(worst));
        return worst;
    }
}

bool
octree_remove(octree *octree, edge e)
{
    if (!octree_in_bounds(octree, e.color))
        return false;
    if (!octree->nodes) {
        for (size_t i = 0; i < octree->count; i++) {
            if ((octree->edges[i].x == e.x) && (octree->edges[i].y == e.y)) {
                octree->edges[i] = octree->edges[--octree->count];
                return true;
            }
        }
    } else {
        for (size_t i = 0; i < 8; i++)
            if (octree_remove(octree->nodes + i, e)) {
                octree->count--;
                if (octree->count <= OCTREE_THRESHOLD)
                    octree_coalesce(octree);
                return true;
            }
    }
    return false;
}

static bool
method_add(struct finder *f, edge e)
{
    return octree_add((octree *)f, e);
}

static bool
method_remove(struct finder *f, edge e)
{
    return octree_remove((octree *)f, e);
}

static float
method_nearest(const struct finder *f, color c, edge *e)
{
    return octree_nearest((const octree *)f, c, e);
}

static void
method_free(const struct finder *f)
{
    octree_free((octree *)f);
    free((struct finder *)f);
}

finder *
octree_create(void)
{
    float high = 1.0f + FLT_EPSILON;
    color bound[2] = {{{0.0f, 0.0f, 0.0f, 0.0f}}, {{high, high, high, high}}};
    struct octree *octree = malloc(sizeof(*octree));
    finder *f = &octree_init(octree, bound)->finder;
    f->add = method_add;
    f->remove = method_remove;
    f->nearest = method_nearest;
    f->free = method_free;
    return f;
}
