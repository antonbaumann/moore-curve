#include <stdio.h>
#include <stdint.h>

#include "svg.c"

int main()
{
    FILE *out_file = fopen("test.svg", "w");
    if (out_file == NULL)
    {
        printf("failed to create file");
    }

    int nr_coords = 4;
    uint64_t x[] = {0, 0, 1, 1};
    uint64_t y[] = {0, 1, 1, 0};

    save_as_svg(x, y, nr_coords, 10, 10, out_file);

    fclose(out_file);
    return 0;
}