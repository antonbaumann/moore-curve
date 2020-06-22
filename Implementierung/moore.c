#include <stdio.h>
#include <stdint.h>

// makes returning two variables more comfortable
struct tuple {
    uint64_t x, y;
};


struct tuple rot(
        uint64_t x,
        uint64_t y,
        uint64_t rx,
        uint64_t ry,
        uint64_t p
) {
    if (ry == 0) {
        if (rx == 1) {
            x = p - 1 - x;
            y = p - 1 - y;
        }
        // vertausche x und y
        uint64_t z = x;
        x = y;
        y = z;
    }
    struct tuple t = {.x = x, .y = y};
    return t;
}

struct tuple m(uint64_t t, uint64_t p) {
    uint64_t x = 0;
    uint64_t y = 0;

    for (uint64_t m = 1; m < p; m *= 2) {
        uint64_t rx = (uint64_t) 1 & (t / 2);
        uint64_t ry = (uint64_t) 1 & (t ^ rx);
        struct tuple pair = rot(x, y, rx, ry, m);
        x = pair.x;
        y = pair.y;
        x += m * rx;
        y += m * ry;
        t /= 4; // zur nächsten Quaternärziffer
    }

    struct tuple pair = {.x = x, .y = y};
    return pair;
}

int moore_c_iterative(long degree, uint64_t *x_coords, uint64_t *y_coords) {
    printf("moore c iterative: degree %ld\n", degree);

    unsigned int shifts = 2 * degree - 1;
    unsigned long long nr_iterations = (unsigned long long) 2 << shifts; // 2 ^ (2 * degree)

    for (uint64_t i = 0; i < nr_iterations; i++) {
        if (i % 10000 == 0) {
            printf("%llu / %llu\r", i, nr_iterations);
        }
        struct tuple pair = m(i, nr_iterations);
        x_coords[i] = pair.x;
        y_coords[i] = pair.y;
    }
    return 0;
}