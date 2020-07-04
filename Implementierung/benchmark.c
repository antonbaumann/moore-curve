#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#include "moore.h"
#include "svg.h"

static const char *BENCHMARK_DIR = "benchmark";

struct benchmark_result {
    double abs_time;
    double avg_time;
};

enum implementation {
    C, ASM
};

// creates benchmark dir if not existing
// returns 0 if successfully created or dir already existing
// otherwise returns -1
int create_benchmark_dir() {
    errno = 0;
    int res = mkdir(BENCHMARK_DIR, 0755);
    if (res == 0) return 0;
    if (errno == EEXIST) return 0;
    return -1;
}

int save_last_result(long degree, enum implementation impl, uint64_t *x_coords, uint64_t *y_coords) {
    char *implementation_description;
    switch (impl) {
        case C:
            implementation_description = "C";
            break;
        case ASM:
            implementation_description = "ASM";
            break;
        default:
            printf("[!] implementation not valid\n");
            return -1;
    }

    char filename[128];
    if (strlen(BENCHMARK_DIR) > 30) {
        printf("[!] benchmark directory name is to long\n");
        return -1;
    }
    // this is save since
    // - len(benchmark_dir) < 30
    // - string representation of a double can't be longer than 20
    // - len(implementation_description) <= 4
    sprintf(filename, "%s/moore_d%ld_%s.svg", BENCHMARK_DIR, degree, implementation_description);

    FILE *svg_file = fopen(filename, "w");
    if (svg_file == NULL) {
        printf("[!] could not open svg file\n");
        return -1;
    }

    save_as_svg(x_coords, y_coords, degree, 5, svg_file);
    return 0;
}

struct benchmark_result benchmark_implementation(
        long degree,
        long repetitions,
        uint64_t *x_coords,
        uint64_t *y_coords,
        void impl(uint64_t, uint64_t *, uint64_t *)
) {
    struct timespec start;
    int err = clock_gettime(CLOCK_MONOTONIC, &start);
    if (err != 0) {
        printf("[!] failed to get current time\n");
        exit(EXIT_FAILURE);
    }

    for (long i = 0; i < repetitions; i++) {
        impl(degree, x_coords, y_coords);
    }

    struct timespec end;
    err = clock_gettime(CLOCK_MONOTONIC, &end);
    if (err != 0) {
        printf("[!] failed to get current time\n");
        exit(EXIT_FAILURE);
    }

    double time = end.tv_sec - start.tv_sec + 1e-9 * (end.tv_nsec - start.tv_nsec);
    double avg_time = time / repetitions;

    struct benchmark_result res = {
            .abs_time=time,
            .avg_time=avg_time,
    };
    return res;
}

void benchmark(long degree, long repetitions) {
    int err = create_benchmark_dir();
    if (err != 0) {
        printf("[!] failed to create benchmark directory\n");
        exit(EXIT_FAILURE);
    }

    // precalculate number of coordinates
    unsigned long shifts = 2 * degree - 1;
    unsigned long long nr_coords = (unsigned long long) 2 << shifts; // 2^(2 * degree)

    // initialize coordinate vectors
    uint64_t *x_coords = malloc(sizeof(uint64_t) * nr_coords);
    uint64_t *y_coords = malloc(sizeof(uint64_t) * nr_coords);

    struct benchmark_result res;

    printf("[i] running assembly implementation %ld times (degree: %ld)\n", repetitions, degree);
    res = benchmark_implementation(
            degree,
            repetitions,
            x_coords,
            y_coords,
            moore
    );
    printf("[i] absolute time: %f\n", res.abs_time);
    printf("[i] average time:  %f\n", res.avg_time);

    err = save_last_result(degree, ASM, x_coords, y_coords);
    if (err != 0) {
        exit(EXIT_FAILURE);
    }


    printf("[i] running c implementation %ld times (degree: %ld)\n", repetitions, degree);
    res = benchmark_implementation(
            degree,
            repetitions,
            x_coords,
            y_coords,
            moore_c_iterative
    );
    printf("[i] absolute time: %f\n", res.abs_time);
    printf("[i] average time:  %f\n", res.avg_time);

    err = save_last_result(degree, C, x_coords, y_coords);
    if (err != 0) {
        exit(EXIT_FAILURE);
    }

    free(x_coords);
    free(y_coords);
}