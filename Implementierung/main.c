#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "svg.c"
#include "main.h"

enum impl_variant {
    ASSEMBLY,
    C_ITERATIVE,
    C_RECURSIVE,
    UNKNOWN,
};

int main(int argc, char **argv) {

    FILE *out_file = fopen("test.svg", "w");
    if (out_file == NULL) {
        printf("failed to create file");
    }

    uint64_t x[] = {0, 0, 1, 1};
    uint64_t y[] = {0, 1, 1, 0};

    save_as_svg(x, y, 4, 10, 10, out_file);

    fclose(out_file);


    enum impl_variant variant = UNKNOWN;
    long degree = 0;

    static struct option long_options[] = {
            {"implementation", required_argument, NULL, 'i'},
            {"degree",         required_argument, NULL, 'd'},
            {NULL,             no_argument,       NULL, 0},
    };

    int c;
    // loop over all of the options
    while ((c = getopt_long(argc, argv, "i:d:", long_options, NULL)) != -1) {
        if (c == -1)break;
        switch (c) {
            case 'i':
                if (strcmp(optarg, "assembly") == 0) variant = ASSEMBLY;
                else if (strcmp(optarg, "c_iterative") == 0) variant = C_ITERATIVE;
                else if (strcmp(optarg, "c_recursive") == 0) variant = C_RECURSIVE;
                else variant = UNKNOWN;
                break;
            case 'd':
                degree = parse_degree(optarg);
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

    // precalculate number of coordinates
    long nr_coords = 4 << degree;

    // initialize coordinate vectors
    uint64_t x_coords[nr_coords];
    uint64_t y_coords[nr_coords];

    switch (variant) {
        case C_ITERATIVE:
            moore_c_iterative(degree, x_coords, y_coords);
            break;
        case C_RECURSIVE:
            moore_c_recursive(degree, x_coords, y_coords);
            break;
        case ASSEMBLY:
            moore_asm(degree, x_coords, y_coords);
            break;
        default:
            printf("this should not have happened\n");
            return 1;
    }
    return 0;
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

void print_help() {
    printf("================Help================\n");
    printf("--help or -h :          print help\n");
    printf("--implementation or -i: specify implementation [c_iterative, c_recursive, assembly]\n");
    printf("--degree or -d:         specify degree of moore curve\n\n");
    printf("Usage: moore -i <implementation> -d <degree>\n");
    printf("====================================\n");
}

// TODO: Assembler anbindung/Implementation
int moore_asm(long degree, uint64_t *x, uint64_t *y) {
    printf("moore assembly: degree %ld\n", degree);
    return 0;
}

//TODO: C Implementation
int moore_c_iterative(long degree, uint64_t *x, uint64_t *y) {
    printf("moore c iterative: degree %ld\n", degree);
    return 0;
}

//TODO: C Implementation
int moore_c_recursive(long degree, uint64_t *x, uint64_t *y) {
    printf("moore c recursive: degree %ld\n", degree);
    return 0;
}