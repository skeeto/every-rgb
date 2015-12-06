#include "finder.h"

#define KDTREE_THRESHOLD 64

enum kdtree_axis { KDTREE_X, KDTREE_Y, KDTREE_Z };

typedef struct kdtree {
    finder finder;
    enum kdtree_axis axis;
    struct kdtree *left, *right;
    long count;
    edge edges[KDTREE_THRESHOLD];
} kdtree;

finder *kdtree_create(void);
