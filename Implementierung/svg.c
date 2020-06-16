#include <stdio.h>
#include <stdint.h>

#include "svg.h"

void save_as_svg(
        const uint64_t *x,
        const uint64_t *y,
        int nr_coordinates,
        int svg_size,
        int scale,
        FILE *output_file
) {
    // define svg components
    char *svg_open_f = "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 %i %i\">";
    char *svg_close = "</svg>";
    char *svg_path_open = "<path d=\"";
    char *svg_path_close = "\" stroke=\"#000\" fill=\"none\"/>\"";

    fprintf(output_file, svg_open_f, svg_size, svg_size);
    fprintf(output_file, "%s", svg_path_open);

    // move origin of path to first coordinate in coord list
    if (nr_coordinates > 0) {
        fprintf(output_file, "M%llu %llu ", x[0] * scale, y[0] * scale);
    }

    // print remaining coordinates to svg file
    for (uint64_t i = 1; i < nr_coordinates; i++) {
        fprintf(output_file, "L%llu %llu ", x[i] * scale, y[i] * scale);
    }

    fprintf(output_file, "%s", svg_path_close);
    fprintf(output_file, "%s", svg_close);
}