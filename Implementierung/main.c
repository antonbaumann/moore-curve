#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "svg.h"
#include "moore.h"
#include "main.h"
#include "benchmark.h"

// Rechnerhalle does not allow us to allocate enough ram to calculate
// moore curve of degree 17
const int MAX_DEGREE = 16;

enum impl_variant {
    ASSEMBLY,
    C_ITERATIVE,
    C_BATCH,
    UNKNOWN,
};

static int benchmark_flag;

int main(int argc, char **argv) {
    enum impl_variant variant = UNKNOWN;
    uint32_t degree = 0;
    uint32_t repetitions = 0;
    char *path = "";
    size_t path_length = 0;

    static struct option long_options[] = {
            {"degree",         required_argument, NULL,      'd'},
            {"implementation", optional_argument, NULL,      'i'},
            {"path",           optional_argument, NULL,      'p'},
            {"repetitions",    optional_argument, NULL,      'r'},
            {"benchmark",      no_argument, &benchmark_flag, 1},
            {NULL,             no_argument,       NULL,      0},
    };

    int c;
    int option_index = 0;
    // loop over all of the options
    while ((c = getopt_long(argc, argv, "i:d:p:r:", long_options, &option_index)) != -1) {
        if (c == -1)break;
        switch (c) {
            case 0:
                break;
            case 'i':
                if (strcmp(optarg, "asm") == 0) variant = ASSEMBLY;
                else if (strcmp(optarg, "c_naive") == 0) variant = C_ITERATIVE;
                else if (strcmp(optarg, "c_batch") == 0) variant = C_BATCH;
                else variant = UNKNOWN;
                break;
            case 'd':
                degree = parse_uint32(optarg);
                break;
            case 'r':
                repetitions = parse_uint32(optarg);
                break;
            case 'p':
                path = optarg;
                path_length = strlen(path);
                break;
            case '?':
            default:
                print_help();
                return EXIT_FAILURE;
        }
    }

    if (degree <= 0 || degree > MAX_DEGREE) {
        printf("[!] invalid argument: degree must be an integer in range [1, ..., %d]\n", MAX_DEGREE);
        print_help();
        return EXIT_FAILURE;
    }

    if (benchmark_flag) {
        if (repetitions <= 0) {
            printf("[!] invalid argument: repetitions must be > 1\n");
            return EXIT_FAILURE;
        }
        benchmark(degree, repetitions);
        return EXIT_SUCCESS;
    }

    if (variant == UNKNOWN) {
        printf("[!] invalid argument: unknown implementation variant specified\n");
        print_help();
        return EXIT_FAILURE;
    }

    if (!(4 <= path_length &&
          !strcmp(path + path_length - 4, ".svg"))) {   // pointer arithmetik "file.svg"\0 und srtlen(".svg") == 4
        printf("[!] invalid argument: filename, filename should resemble \"file.svg\"\n");
        print_help();
        return EXIT_FAILURE;
    }

    // precalculate number of coordinates
    uint32_t shifts = 2 * degree - 1;
    uint64_t nr_coords = (uint64_t) 2 << shifts; // 2^(2 * degree)


    // initialize coordinate vectors
    uint32_t *x_coords = malloc(sizeof(uint32_t) * nr_coords);
    uint32_t *y_coords = malloc(sizeof(uint32_t) * nr_coords);
    if (x_coords == NULL || y_coords == NULL) {
        printf("Allocation failed...\n");
        return EXIT_FAILURE;
    }

    switch (variant) {
        case C_ITERATIVE:
            moore_c_naive_wrapper(degree, x_coords, y_coords);
            break;
        case C_BATCH:
            moore_c_batch_wrapper(degree, x_coords, y_coords);
            break;
        case ASSEMBLY:
            moore_asm_wrapper(degree, x_coords, y_coords);
            break;
        default:
            printf("this should not have happened\n");
            return 1;
    }

    int err = write_svg(path, x_coords, y_coords, degree);

    free(x_coords);
    free(y_coords);
    return err;
}

// parses argument for --degree flag
// if argument is invalid return 0
uint32_t parse_uint32(char *str) {
    errno = 0;  // reset errno to 0 before call of strtol
    char *endptr = NULL;
    long deg = strtol(str, &endptr, 10);

    // argument does not start with digit
    if (str == endptr) return -1;

    // underflow occurred
    if (errno == ERANGE && deg == LONG_MIN) return -1;

    // overflow occurred
    if (errno == ERANGE && deg == LONG_MAX) return -1;

    // error if deg > 2^32
    if (deg > UINT32_MAX) return -1;

    return deg <= 0 ? 0 : deg;
}

int write_svg(char *path, uint32_t *x_coords, uint32_t *y_coords, unsigned int degree) {
    FILE *out_file = fopen(path, "w");
    if (out_file == NULL) {
        printf("failed to create file\n");
        return 1;
    }

    printf("writing data...\n");

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
    printf("====================================Help=======================================\n");
    printf("Usage for printing a SVG-File: moore -i <implementation> -d <degree> -p <path>\n");
    printf("Usage for benchmarking:        moore --benchmark -d <degree> -r <repetitions> \n");
    printf("================================Flag Description===============================\n");
    printf("--help or -h :          print help\n");
    printf("--implementation or -i: specify implementation [c_naive, c_batch, asm]\n");
    printf("--degree or -d:         specify degree of moore curve\n");
    printf("--path or -p:           specify path for SVG-File\n");
    printf("--repetitions or -r:    specify amount of repetitions for benchmarking\n");
    printf("===============================================================================\n");
}

void moore_asm_wrapper(uint32_t degree, uint32_t *x, uint32_t *y) {
    printf("moore assembly: degree %u\n", degree);
    moore(degree, x, y);
    printf("done!\n");
}

void moore_c_naive_wrapper(uint32_t degree, uint32_t *x, uint32_t *y) {
    printf("c naive: degree %u\n", degree);
    moore_c_iterative(degree, x, y);
    printf("done!\n");
}

void moore_c_batch_wrapper(uint32_t degree, uint32_t *x, uint32_t *y) {
    printf("c batch: degree %u\n", degree);
    moore_c_batch(degree, x, y);
    printf("done!\n");
}
