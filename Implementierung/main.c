#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>

#include "svg.c"
#include "moore.c"
#include "main.h"

enum impl_variant {
    ASSEMBLY,
    C_ITERATIVE,
    UNKNOWN,
};

int main(int argc, char **argv) {
    enum impl_variant variant = UNKNOWN;
    long degree = 0;
    char *path;
    size_t path_length;

    static struct option long_options[] = {
            {"implementation", required_argument, NULL, 'i'},
            {"degree",         required_argument, NULL, 'd'},
            {"path",           required_argument, NULL, 'p'},
            {NULL,             no_argument,       NULL, 0},
    };

    int c;
    // loop over all of the options
    while ((c = getopt_long(argc, argv, "i:d:p:", long_options, NULL)) != -1) {
        if (c == -1)break;
        switch (c) {
            case 'i':
                if (strcmp(optarg, "assembly") == 0) variant = ASSEMBLY;
                else if (strcmp(optarg, "c") == 0) variant = C_ITERATIVE;
                else variant = UNKNOWN;
                break;
            case 'd':
                degree = parse_degree(optarg);
                break;
            case 'p':
                path = optarg;
                path_length = strlen(path);
                break;
            default:
                print_help();
                break;
        }
    }

    if (degree <= 0 || degree > 20) {
        printf("[!] invalid argument: degree must be an integer in range [1, ..., 20]\n");
        print_help();
        return 1;
    }

    if (variant == UNKNOWN) {
        printf("[!] invalid argument: unknown implementation variant specified\n");
        print_help();
        return 1;
    }


    if (!(4 <= path_length && !strcmp (path + path_length - 4, ".svg"))){   // pointer arithmetik "file.svg"\0 und srtlen(".svg") == 4
        printf("[!] invalid argument: filename, filename should resemble \"file.svg\"\n");
        print_help();
        return 1;
    }

    // precalculate number of coordinates
    unsigned int shifts = 2 * degree - 1;
    unsigned long long nr_coords = (unsigned long long) 2 << shifts; // 2^(2 * degree)


    // initialize coordinate vectors
    uint64_t *x_coords = malloc(sizeof(uint64_t) * nr_coords);
    uint64_t *y_coords = malloc(sizeof(uint64_t) * nr_coords);
    if (x_coords == NULL || y_coords == NULL) {
        printf("Allocation failed...\n");
        return EXIT_FAILURE;
    }

    switch (variant) {
        case C_ITERATIVE:
            moore_c_iterative((uint64_t) degree, x_coords, y_coords);
            break;
        case ASSEMBLY:
            moore_asm(degree, x_coords, y_coords);
            break;
        default:
            printf("this should not have happened\n");
            return 1;
    }
//
    int err = write_svg(path, x_coords, y_coords, degree);
    free(x_coords);
    free(y_coords);
    return err;
}

// parses argument for --degree flag
// if argument is invalid return 0
long parse_degree(char *str) {
    errno = 0;  // reset errno to 0 before call of strtol
    char *endptr = NULL;
    long deg = strtol(str, &endptr, 10);

    // argument does not start with digit
    if (str == endptr) return -1;

    // underflow occurred
    if (errno == ERANGE && deg == LONG_MIN) return -1;

    // overflow occurred
    if (errno == ERANGE && deg == LONG_MAX) return -1;

    return deg <= 0 ? -1 : deg;
}

int write_svg(char *path, uint64_t *x_coords, uint64_t *y_coords, unsigned int degree) {
    FILE *out_file = fopen(path, "w");
    if (out_file == NULL) {
        printf("failed to create file\n");
        return 1;
    }
    save_as_svg(
            x_coords,
            y_coords,
            degree,
            5,
            out_file
    );

    int err = fclose(out_file);
    if (err != 0) {
        printf("failed to close file\n");
        return err;
    }
    return 0;
}

void print_help() {
    printf("================Help================\n");
    printf("--help or -h :          print help\n");
    printf("--implementation or -i: specify implementation [c, assembly]\n");
    printf("--degree or -d:         specify degree of moore curve\n");
    printf("--path or -p:           specify path for SVG-File\n");
    printf("Usage: moore -i <implementation> -d <degree> -p <path>\n");
    printf("====================================\n");
}

// TODO: Assembler anbindung/Implementation
void moore_asm(long degree, uint64_t *x, uint64_t *y) {
    printf("moore assembly: degree %ld\n", degree);
    moore((uint64_t) degree, x, y);
}
