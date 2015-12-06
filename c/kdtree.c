#include <stdlib.h>
#include <assert.h>
#include "kdtree.h"

static inline bool
kdtree_is_leaf(const kdtree *k)
{
    return k->left == NULL;
}

static kdtree *
kdtree_subcreate(enum kdtree_axis axis)
{
    kdtree *k = malloc(sizeof(*k));
    k->axis = axis;
    k->left = k->right = NULL;
    k->count = 0;
    return k;
}

static int
edge_cmp(enum kdtree_axis a, const edge *restrict e0, const edge *restrict e1)
{
    float c0[3];
    float c1[3];
    for (int i = 0; i < 3; i++) {
        c0[i] = e0->color.c[(i + a) % 3];
        c1[i] = e1->color.c[(i + a) % 3];
    }
    if (c0[0] == c1[0]) {
        if (c0[1] == c1[1]) {
            if (c0[2] == c1[2]) {
                return 0;
            } else {
                return c1[2] < c0[2] ? 1 : -1;
            }
        } else {
            return c1[1] < c0[1] ? 1 : -1;
        }
    } else {
        return c1[0] < c0[0] ? 1 : -1;
    }
}

static int
cmpx(const void *a, const void *b)
{
    return edge_cmp(KDTREE_X, a, b);
}

static int
cmpy(const void *a, const void *b)
{
    return edge_cmp(KDTREE_Y, a, b);

}

static int
cmpz(const void *a, const void *b)
{
    return edge_cmp(KDTREE_Z, a, b);
}

static int (*cmp[])(const void *a, const void *b) = {cmpx, cmpy, cmpz};

static bool kdtree_add(kdtree *k, edge e);

static void
kstree_split(kdtree *k)
{
    qsort(k->edges, k->count, sizeof(k->edges[0]), cmp[k->axis]);
    enum kdtree_axis axis = (k->axis + 1) % 3;
    k->left = kdtree_subcreate(axis);
    k->right = kdtree_subcreate(axis);
    for (long i = 0; i < k->count; i++) {
        if (i <= k->count / 2)
            kdtree_add(k->left, k->edges[i]);
        else
            kdtree_add(k->right, k->edges[i]);
    }
    k->edges[0] = k->edges[k->count / 2];
}

static bool
kdtree_add(kdtree *k, edge e)
{
    if (!kdtree_is_leaf(k)) {
        int result = edge_cmp(k->axis, &e, &k->edges[0]);
        k->count++;
        bool added = kdtree_add(result <= 0 ? k->left : k->right, e);
        assert(added);
        return added;
    } else if (k->count == KDTREE_THRESHOLD) {
        kstree_split(k);
        return kdtree_add(k, e);
    } else {
        assert(k->count < KDTREE_THRESHOLD);
        k->edges[k->count++] = e;
        return true;
    }
}

static bool
kdtree_remove(kdtree *k, edge e)
{
    if (!kdtree_is_leaf(k)) {
        int result = edge_cmp(k->axis, &e, &k->edges[0]);
        k->count--;
        bool removed = kdtree_remove(result <= 0 ? k->left : k->right, e);
        assert(removed);
        return removed;
    } else {
        assert(k->count <= KDTREE_THRESHOLD);
        for (long i = 0; i < k->count; i++) {
            if (e.x == k->edges[i].x && e.y == k->edges[i].y) {
                k->edges[i] = k->edges[--k->count];
                return true;
            }
        }
        return false;
    }
}

static float
kdtree_nearest(const kdtree *k, color c, edge *e)
{
    if (!kdtree_is_leaf(k)) {
        color median = k->edges[0].color;
        int result = edge_cmp(k->axis, &(edge){0, 0, c}, &k->edges[0]);
        kdtree *k0 = result <= 0 ? k->left : k->right;
        float dist0 = kdtree_nearest(k0, c, e);
        float median_dist = fabsf(e->color.c[k->axis] - median.c[k->axis]);
        if (dist0 >= median_dist) {
            kdtree *k1 = result <= 0 ? k->right : k->left;
            edge candidate;
            float dist1 = kdtree_nearest(k1, c, &candidate);
            if (dist1 < dist0) {
                *e = candidate;
                return dist1;
            }
        }
        return dist0;
    } else if (k->count == 0) {
        return INFINITY;
    } else {
        *e = k->edges[0];
        float best2 = color_dist2(c, e->color);
        for (long i = 1; i < k->count; i++) {
            float dist2 = color_dist2(c, k->edges[i].color);
            if (dist2 < best2) {
                best2 = dist2;
                *e = k->edges[i];
            }
        }
        return sqrt(best2);
    }
}

static bool
method_add(finder *f, edge e)
{
    kdtree *k = (kdtree *)f;
    return kdtree_add(k, e);
}

static bool
method_remove(finder *f, edge e)
{
    kdtree *k = (kdtree *)f;
    return kdtree_remove(k, e);
}

static float
method_nearest(const finder *f, color c, edge *e)
{
    const kdtree *k = (const kdtree *)f;
    return kdtree_nearest(k, c, e);
}

static void
method_free(const finder *f)
{
    kdtree *k = (kdtree *)f;
    if (!kdtree_is_leaf(k)) {
        method_free(&k->left->finder);
        method_free(&k->right->finder);
    }
    free(k);
}

finder *
kdtree_create(void)
{
    kdtree *k = kdtree_subcreate(KDTREE_X);
    k->finder.add = method_add;
    k->finder.remove = method_remove;
    k->finder.nearest = method_nearest;
    k->finder.free = method_free;
    return &k->finder;
}
