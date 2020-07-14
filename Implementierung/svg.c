#include <stdio.h>
#include <stdint.h>

#include "svg.h"

void save_as_svg(
        const uint32_t *x,
        const uint32_t *y,
        uint32_t degree,
        uint32_t scale,
        FILE *output_file
) {
    // define svg components
    char *svg_open_f = "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 %d %d\">";
    char *svg_close = "</svg>";
    char *svg_path_open = "<path d=\"";
    char *svg_path_close = "\" stroke=\"#000\" fill=\"none\"/>\"";

    uint32_t side_length = (uint32_t) 2 << (degree - 1);

    uint32_t padding = scale;
    uint64_t nr_coordinates = side_length * side_length;
    uint64_t svg_size = side_length * scale + padding;

    fprintf(output_file, svg_open_f, svg_size, svg_size);
    fprintf(output_file, "%s", svg_path_open);

    // move origin of path to first coordinate in coord list
    if (nr_coordinates > 0) {
        fprintf(output_file, "M%d %llu ", x[0] * scale + padding, svg_size - (uint64_t) (y[0] * scale + padding));
    }
    
    // print remaining coordinates to svg file
    for (uint64_t i = 1; i < nr_coordinates; i++) {
        fprintf(output_file, "L%d %llu ", x[i] * scale + padding, svg_size - (uint64_t) (y[i] * scale + padding));
    }

    fprintf(output_file, "%s", svg_path_close);
    fprintf(output_file, "%s", svg_close);
}