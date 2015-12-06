#pragma once

#include "color.h"
#include "finder.h"

#define OCTREE_THRESHOLD 32

typedef struct octree {
    finder finder;
    color bound[2];
    struct octree *nodes;
    size_t count;
    edge edges[OCTREE_THRESHOLD];
} octree;

finder *octree_create(void);
void    octree_free(const octree *);
bool    octree_add(octree *, edge);
float   octree_nearest(const octree *, color, edge *);
bool    octree_remove(octree *, edge);
