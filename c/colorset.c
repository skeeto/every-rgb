#include <stdlib.h>
#include "colorset.h"
#include "rand.h"

colorset *
colorset_create(int depth, float gamma)
{
    size_t count = 1 << (3 * depth);
    colorset *set;
    size_t size = sizeof(*set) + count * sizeof(set->colors[0]);
    set = malloc(size);
    set->depth = depth;
    set->count = 0;
    int bits = 1 << depth;
    float den = bits - 1;
    for (int r = 0; r < bits; r++)
        for (int g = 0; g < bits; g++)
            for (int b = 0; b < bits; b++)
                set->colors[set->count++] =
                    COLOR(r / den, g / den, b / den, gamma);
    return set;
}

void
colorset_shuffle(colorset *set, uint64_t *state)
{
    for (size_t i = set->count - 1; i > 0; i--) {
        int j = xorshift(state) % (i + 1);
        color tmp = set->colors[i];
        set->colors[i] = set->colors[j];
        set->colors[j] = tmp;
    }
}

static int
cmp(const void *a, const void *b)
{
    return color_cmp(*(color *)a, *(color *)b);
}

void
colorset_sort(colorset *set)
{
    qsort(set->colors, set->count, sizeof(set->colors[0]), cmp);
}

color
colorset_pop(colorset *set)
{
    return set->colors[--set->count];
}

void
colorset_free(const colorset *set)
{
    free((void *)set);
}
