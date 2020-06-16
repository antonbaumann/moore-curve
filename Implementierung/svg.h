#include <stdint.h>
#include <stdio.h>

void save_as_svg(
        const uint64_t *x,
        const uint64_t *y,
        int nr_coordinates,
        int svg_size,
        int scale,
        FILE *output_file
);