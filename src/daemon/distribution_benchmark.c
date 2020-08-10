#define _POSIX_C_SOURCE 199309L
#include "distribution.h"
#include <stdio.h>
#include <time.h>

int main(int argc, char **argv) {
    int iterations = 20000000;
    if (argc < 2) {
        printf("No bucket number found.\n");
        exit(EXIT_FAILURE);
    }

    int buckets_number = atoi(argv[1]);
    double buckets_size = 25;
    printf("%d, ", buckets_number);

    distribution_t *dist_test = distribution_new_linear(buckets_number, buckets_size);
    struct timespec start_update;
    clock_gettime(CLOCK_MONOTONIC, &start_update);
    for(size_t i = 0; i < iterations; i++) {
        distribution_update(dist_test, rand() % (buckets_number * (int) buckets_size + (int) buckets_size + 1));
    }
    struct timespec end_update;
    clock_gettime(CLOCK_MONOTONIC, &end_update);
    double seconds_elapsed_update = end_update.tv_sec - start_update.tv_sec + 1e-9 * (end_update.tv_nsec - start_update.tv_nsec);
    printf("%f, ", seconds_elapsed_update);

    struct timespec start_percentile;
    clock_gettime(CLOCK_MONOTONIC, &start_percentile);
    for(size_t i = 0; i < iterations; i++) {
        distribution_percentile(dist_test, rand() % (100 + 1));
    }
    struct timespec end_percentile;
    clock_gettime(CLOCK_MONOTONIC, &end_percentile);
    double seconds_elapsed_percentile = end_percentile.tv_sec - start_percentile.tv_sec + 1e-9 * (end_percentile.tv_nsec - start_percentile.tv_nsec);
    printf("%f, ", seconds_elapsed_percentile);

    struct timespec start_mixed;
    clock_gettime(CLOCK_MONOTONIC, &start_mixed);
    for(size_t i = 0; i < iterations; i++) {
        i%10 == 0 ? distribution_percentile(dist_test, rand() % (100 + 1)) : distribution_update(dist_test, rand() % (800 + 1));
    }
    struct timespec end_mixed;
    clock_gettime(CLOCK_MONOTONIC, &end_mixed);
    double seconds_elapsed_mixed = end_mixed.tv_sec - start_mixed.tv_sec + 1e-9 * (end_mixed.tv_nsec - start_mixed.tv_nsec);
    printf("%f\n", seconds_elapsed_mixed);
    return 0;
}