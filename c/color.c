#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "octree.h"
#include "kdtree.h"
#include "naive.h"
#include "image.h"
#include "rand.h"
#include "color.h"
#include "colorset.h"

enum method { METHOD_NAIVE, METHOD_OCTREE, METHOD_KDTREE };

static void
print_usage(const char *name, FILE *o)
{
    fprintf(o, "Usage: %s [options]\n", name);
    fprintf(o, "  -o <file>     output file (stdout)\n");
    fprintf(o, "  -s <w:h:d>    image width (512x512x6)\n");
    fprintf(o, "  -S <seed>     select a specific random seed\n");
    fprintf(o, "  -n            steps between video frames (0)\n");
    fprintf(o, "  -p <x,y>      add a start point, may be repeated\n");
    fprintf(o, "  -N            use naive color matcher\n");
    fprintf(o, "  -O            use octree color matcher\n");
    fprintf(o, "  -K            use kdtree color matcher (default)\n");
    fprintf(o, "  -g <gamma>    select gamma (2.2)\n");
    fprintf(o, "  -v            verbose\n");
    fprintf(o, "  -h            print this help\n");
}

int
main(int argc, char **argv)
{
    /* Options */
    FILE *output = stdout;
    uint64_t seed = 0;
    uint32_t width = 512;
    uint32_t height = 512;
    int depth = 6;
    enum method method = METHOD_KDTREE;
    bool verbose = false;
    int steps = 0;
    int nstarts = 0;
    float gamma = 2.2f;
    struct {
        uint32_t x, y;
    } starts[128];

    int option;
    while ((option = getopt(argc, argv, "o:s:S:n:p:g:NOKhv")) != -1) {
        switch (option) {
            case 'o':
                if (strcmp(optarg, "-") != 0) {
                    output = fopen(optarg, "wb");
                    if (output == NULL) {
                        perror(optarg);
                        exit(EXIT_FAILURE);
                    }
                }
                break;
            case 's': {
                char *p = optarg;
                width = strtol(p, &p, 10);
                height = strtol(p + 1, &p, 10);
                depth = strtol(p + 1, &p, 10);
            } break;
            case 'S':
                seed = strtol(optarg, NULL, 16);
                break;
            case 'n':
                steps = atoi(optarg);
                break;
            case 'p': {
                char *p = optarg;
                starts[nstarts].x = strtol(p, &p, 10);
                starts[nstarts].y = strtol(p + 1, &p, 10);
                nstarts++;
            } break;
            case 'g':
                gamma = strtof(optarg, NULL);
                break;
            case 'N':
                method = METHOD_NAIVE;
                break;
            case 'O':
                method = METHOD_OCTREE;
                break;
            case 'K':
                method = METHOD_KDTREE;
                break;
            case 'v':
                verbose = true;
                break;
            case 'h':
                print_usage(argv[0], stdout);
                exit(EXIT_SUCCESS);
                break;
            default:
                print_usage(argv[0], stderr);
                exit(EXIT_FAILURE);
        }
    }
    if (seed == 0)
        seed = seedgen();

    finder *finder;
    switch (method) {
        case METHOD_NAIVE:
            finder = naive_create();
            break;
        case METHOD_OCTREE:
            finder = octree_create();
            break;
        case METHOD_KDTREE:
            finder = kdtree_create();
            break;
    }
    image *image = image_create(width, height);
    colorset *colorset = colorset_create(depth, gamma);
    colorset_shuffle(colorset, &seed);

    if (nstarts == 0) {
        starts[0].x = image->width / 2;
        starts[0].y = image->height / 2;
        nstarts++;
    }

    for (int i = 0; i < nstarts; i++) {
        edge start = {
            .x = starts[i].x,
            .y = starts[i].y,
            .color = colorset_pop(colorset)
        };
        image_set(image, start.x, start.y, start.color);
        finder_add(finder, start);
    }
    size_t pixels_left = image->width * image->height - nstarts;
    while (colorset->count > 0 && pixels_left > 0) {
        if (verbose && colorset->count % 4096 == 0)
            fprintf(stderr, "%zu colors remaining\n", colorset->count);
        if (steps > 0 && colorset->count % steps == 0)
            image_save(image, gamma, output);
        color next_color = colorset_pop(colorset);
        int count = 0;
        do {
            edge target;
            finder_nearest(finder, next_color, &target);
            edge border[8];
            for (int y = -1; y <= 1; y++) {
                for (int x = -1; x <= 1; x++) {
                    uint32_t tx = target.x + x;
                    uint32_t ty = target.y + y;
                    if (image_get(image, tx, ty).a == 0)
                        border[count++] = (edge){tx, ty, next_color};
                }
            }
            if (count > 0) {
                edge result = border[xorshift(&seed) % count];
                image_set(image, result.x, result.y, result.color);
                pixels_left--;
                finder_add(finder, result);
            } else {
                finder_remove(finder, target);
            }
        } while (count == 0);
    }

    image_save(image, gamma, output);
    colorset_free(colorset);
    image_free(image);
    finder_free(finder);
    return 0;
}
