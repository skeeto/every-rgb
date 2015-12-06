#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "rand.h"

uint64_t
xorshift(uint64_t *state)
{
    uint64_t x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * UINT64_C(2685821657736338717);
}

uint64_t
seedgen(void)
{
    uint64_t seed = 0;
    FILE *urandom = fopen("/dev/urandom", "rb");
    int r = fread(&seed, sizeof(seed), 1, urandom);
    (void) r;
    assert(r == 1);
    fclose(urandom);
    return seed;
}
