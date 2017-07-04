#pragma once

#include <stdio.h>
#include <math.h>

#define COLOR(r, g, b, gamma) \
    (color){{powf(r, gamma), powf(g, gamma), powf(b, gamma), 1.0f}}

typedef union color {
    struct {
        float r, g, b, a;
    } p;
    float c[4];
} color;

static inline int
color_cmp(color a, color b)
{
    if (a.p.r == b.p.r) {
        if (a.p.g == b.p.g) {
            if (a.p.b == b.p.b) {
                return 0;
            } else {
                return a.p.b < b.p.b ? -1 : 1;
            }
        } else {
            return a.p.g < b.p.g ? -1 : 1;
        }
    } else {
        return a.p.r < b.p.r ? -1 : 1;
    }
}

static inline float
color_dist2(color c0, color c1)
{
    float dr = c0.p.r - c1.p.r;
    float dg = c0.p.g - c1.p.g;
    float db = c0.p.b - c1.p.b;
    return dr * dr + dg * dg + db * db;
}

static inline void
color_print(color color, FILE *out)
{
    fprintf(out, "(%.2f %.2f %.2f)\n", color.p.r, color.p.g, color.p.b);
}
