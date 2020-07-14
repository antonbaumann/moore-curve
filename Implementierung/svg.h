#include <stdint.h>
#include <stdio.h>

void save_as_svg(
        const uint32_t *x,
        const uint32_t *y,
        uint32_t degree,
        uint32_t scale,
        FILE *output_file
);