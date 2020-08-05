#define _POSIX_C_SOURCE 199309L
#include "distribution.h"
#include <stdio.h>
#include <time.h>

int main(int argc, char **argv) {
    
    distribution_t *dist_test = distribution_new_linear(30, 25);
    int iterations = 10000000;
    struct timespec start_update;
    clock_gettime(CLOCK_MONOTONIC, &start_update);
    for(size_t i = 0; i < iterations; i++) {
        distribution_update(dist_test, rand() % (800 + 1));
    }
    struct timespec end_update;
    clock_gettime(CLOCK_MONOTONIC, &end_update);
    double seconds_elapsed_update = end_update.tv_sec - start_update.tv_sec + 1e-9 * (end_update.tv_nsec - start_update.tv_nsec);
    printf("Update done after %f seconds\n", seconds_elapsed_update);

    struct timespec start_percentile;
    clock_gettime(CLOCK_MONOTONIC, &start_percentile);
    for(size_t i = 0; i < iterations; i++) {
        distribution_percentile(dist_test, rand() % (100 + 1));
    }
    struct timespec end_percentile;
    clock_gettime(CLOCK_MONOTONIC, &end_percentile);
    double seconds_elapsed_percentile = end_percentile.tv_sec - start_percentile.tv_sec + 1e-9 * (end_percentile.tv_nsec - start_percentile.tv_nsec);
    printf("Percentile done after %f seconds\n", seconds_elapsed_percentile);

    struct timespec start_mixed;
    clock_gettime(CLOCK_MONOTONIC, &start_mixed);
    for(size_t i = 0; i < iterations; i++) {
        i%10 == 0 ? distribution_percentile(dist_test, rand() % (100 + 1)) : distribution_update(dist_test, rand() % (800 + 1));
    }
    struct timespec end_mixed;
    clock_gettime(CLOCK_MONOTONIC, &end_mixed);
    double seconds_elapsed_mixed = end_mixed.tv_sec - start_mixed.tv_sec + 1e-9 * (end_mixed.tv_nsec - start_mixed.tv_nsec);
    printf("Update and Percentile mixed done after %f seconds\n", seconds_elapsed_mixed);
    return 0;
}