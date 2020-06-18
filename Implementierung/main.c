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

    int nr_coords = 4;
    uint64_t x[] = {0, 0, 1, 1};
    uint64_t y[] = {0, 1, 1, 0};

    save_as_svg(x, y, nr_coords, 10, 10, out_file);

    fclose(out_file);


    enum impl_variant variant = UNKNOWN;
    int degree = -1;

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

    if (degree < 0) {
        printf("[!] invalid argument: degree must be a positive integer\n");
        print_help();
        return 1;
    }

    if (variant == UNKNOWN) {
        printf("[!] invalid argument: unknown implementation variant specified\n");
        print_help();
        return 1;
    }

    switch (variant) {
        case C_ITERATIVE:
            moore_c_iterative(degree);
            break;
        case C_RECURSIVE:
            moore_c_recursive(degree);
            break;
        case ASSEMBLY:
            moore_asm(degree);
            break;
        default:
            printf("this should not have happened\n");
            return 1;
    }
    return 0;
}

// parses argument for --degree flag
// if argument is invalid return -1
int parse_degree(char *str) {
    errno = 0;  // reset errno to 0 before call of strtol
    char *endptr = NULL;
    long deg = strtol(str, &endptr, 10);

    // argument does not start with digit
    if (str == endptr) return -1;

    // underflow occurred
    if (errno == ERANGE && deg == LONG_MIN) return -1;

    // overflow occurred
    if (errno == ERANGE && deg == LONG_MAX) return -1;

    // check if number is positive and fits into an integer
    if (deg < 0 || deg > INT_MAX) return -1;

    return (int) deg;
}

void print_help() {
    printf("================Help================\n");
    printf("--help or -h :          print help\n");
    printf("--implementation or -i: specify implementation [c_iterative, c_recursive, assembly]\n");
    printf("--degree or -d:         specify degree of moore curve\n\n");
    printf("Usage: moore -i <implementation> -d <degree>\n");
    printf("====================================\n");
    exit(2);
}

// TODO: Assembler anbindung/Implementation
int moore_asm(int n) {
    printf("moore assembly: degree %d", n);
    return 0;
}

//TODO: C Implementation
int moore_c_iterative(int n) {
    printf("moore c iterative: degree %d", n);
    return 0;
}

//TODO: C Implementation
int moore_c_recursive(int n) {
    printf("moore c recursive: degree %d", n);
    return 0;
}