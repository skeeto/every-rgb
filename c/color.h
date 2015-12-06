#pragma once

#include <stdio.h>
#include <math.h>

#define COLOR(r, g, b, gamma) \
    (color){{powf(r, gamma), powf(g, gamma), powf(b, gamma), 1.0f}}

typedef union color {
    struct {
        float r, g, b, a;
    };
    float c[4];
} color;

static inline int
color_cmp(color a, color b)
{
    if (a.r == b.r) {
        if (a.g == b.g) {
            if (a.b == b.b) {
                return 0;
            } else {
                return a.b < b.b ? -1 : 1;
            }
        } else {
            return a.g < b.g ? -1 : 1;
        }
    } else {
        return a.r < b.r ? -1 : 1;
    }
}

static inline float
color_dist2(color c0, color c1)
{
    float dr = c0.r - c1.r;
    float dg = c0.g - c1.g;
    float db = c0.b - c1.b;
    return dr * dr + dg * dg + db * db;
}

static inline void
color_print(color color, FILE *out)
{
    fprintf(out, "(%.2f %.2f %.2f)\n", color.r, color.g, color.b);
}
