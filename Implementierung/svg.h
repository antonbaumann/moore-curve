#include <stdint.h>
#include <stdio.h>

void save_as_svg(
        const uint32_t *x,
        const uint32_t *y,
        unsigned int degree,
        unsigned int scale,
        FILE *output_file
);