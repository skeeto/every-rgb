#pragma once

#include "finder.h"

typedef struct naive {
    finder finder;
    size_t count, max;
    edge *edges;
} naive;

finder *naive_create(void);
void    naive_free(const naive *);
bool    naive_add(naive *, edge);
bool    naive_remove(naive *, edge);
float   naive_nearest(const naive *, color, edge *);
