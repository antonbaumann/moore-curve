#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "moore.h"
#include "svg.h"

// name of directory where svg files from benchmark runs are stored
// to check if benchmarked function produced correct result
static const char *BENCHMARK_DIR = "benchmark";

struct benchmark_result {
    long abs_time_ns;
    long avg_time_ns;
};

enum implementation {
    C, ASM, ASM_AVX, C_BATCH
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

// saves result from last iteration as svg
// also handles naming of file
int save_last_result(long degree, enum implementation impl, uint32_t *x_coords, uint32_t *y_coords) {
    char *implementation_description;
    switch (impl) {
        case C:
            implementation_description = "C";
            break;
        case ASM:
            implementation_description = "ASM";
            break;
        case ASM_AVX:
            implementation_description = "ASM_AVX";
            break;
        case C_BATCH:
            implementation_description = "C_BATCH";
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
    // - len(benchmark_dir) <= 30
    // - string representation of a long can't be longer than 20
    // - len(implementation_description) <= 4
    sprintf(filename, "%s/moore_d%ld_%s.svg", BENCHMARK_DIR, degree, implementation_description);

    FILE *svg_file = fopen(filename, "w");
    if (svg_file == NULL) {
        printf("[!] could not open svg file\n");
        return -1;
    }

    printf("[i] writing last result to disk ...\n");
    save_as_svg(x_coords, y_coords, degree, 5, svg_file);

    int err = fclose(svg_file);
    if (err != 0) {
        printf("[!] failed to close file\n");
    }
    return err;
}

// utility function that pretty prints benchmarking result
void print_result(struct benchmark_result res) {
    double seconds = res.abs_time_ns * 1e-9;

    printf("[i] absolute time: %fs\n", seconds);
    printf("[i] average time:  %ldns\n", res.avg_time_ns);
}

// takes a implementation of a moore curve generating function
// and runs it `repetitions` times
// returns the benchmark result
struct benchmark_result benchmark_implementation(
        long degree,
        long repetitions,
        uint32_t *x_coords,
        uint32_t *y_coords,
        void impl(uint32_t, uint32_t *, uint32_t *)
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

    long time_ns = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    long avg_time_ns = time_ns / repetitions;

    struct benchmark_result res = {
            .abs_time_ns=time_ns,
            .avg_time_ns=avg_time_ns,
    };
    return res;
}

// run benchmark of all supported implementations `repetitions` times with `degree`
// writes result of last repetition to benchmark directory if `write_result` is true
void benchmark(uint32_t degree, uint32_t repetitions, uint32_t write_result) {
    int err = create_benchmark_dir();
    if (err != 0) {
        printf("[!] failed to create benchmark directory\n");
        exit(EXIT_FAILURE);
    }

    // precalculate number of coordinates
    uint64_t shifts = 2 * degree - 1;
    uint64_t nr_coords = (uint64_t) 2 << shifts; // 2^(2 * degree)

    // initialize coordinate vectors
    uint32_t *x_coords = malloc(sizeof(uint32_t) * nr_coords);
    if (x_coords == NULL) {
        printf("Allocation failed...\n");
        exit(EXIT_FAILURE);
    }
    uint32_t *y_coords = malloc(sizeof(uint32_t) * nr_coords);
    if (y_coords == NULL) {
        printf("Allocation failed...\n");
        exit(EXIT_FAILURE);
    }

    struct benchmark_result res;

    // only run avx2 implementation if supported on machine
    #ifdef __AVX2__
    printf("[i] running assembly avx implementation %d times (degree: %d)\n", repetitions, degree);
    res = benchmark_implementation(
            degree,
            repetitions,
            x_coords,
            y_coords,
            moore_avx
    );
    print_result(res);

    // wait a bit before benchmarking next implementation
    sleep(3);

    if (write_result) {
        err = save_last_result(degree, ASM_AVX, x_coords, y_coords);
        if (err != 0) {
            exit(EXIT_FAILURE);
        }
    }
    #endif

    printf("[i] running assembly (without avx) implementation %d times (degree: %d)\n", repetitions, degree);
    res = benchmark_implementation(
            degree,
            repetitions,
            x_coords,
            y_coords,
            moore
    );
    print_result(res);
    sleep(3);

    if (write_result) {
        err = save_last_result(degree, ASM, x_coords, y_coords);
        if (err != 0) {
            exit(EXIT_FAILURE);
        }
    }

    printf("[i] running c_batch implementation %d times (degree: %d)\n", repetitions, degree);
    res = benchmark_implementation(
            degree,
            repetitions,
            x_coords,
            y_coords,
            moore_c_batch
    );
    print_result(res);
    sleep(3);

    if (write_result) {
        err = save_last_result(degree, C_BATCH, x_coords, y_coords);
        if (err != 0) {
            exit(EXIT_FAILURE);
        }
    }

    printf("[i] running c_naive implementation %d times (degree: %d)\n", repetitions, degree);
    res = benchmark_implementation(
            degree,
            repetitions,
            x_coords,
            y_coords,
            moore_c_iterative
    );
    print_result(res);

    if (write_result) {
        err = save_last_result(degree, C, x_coords, y_coords);
        if (err != 0) {
            exit(EXIT_FAILURE);
        }
    }

    free(x_coords);
    free(y_coords);
}
