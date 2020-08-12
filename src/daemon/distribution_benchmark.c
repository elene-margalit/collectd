#define _POSIX_C_SOURCE 199309L
#include "distribution.h"
#include <stdio.h>
#include <time.h>

double calculate_gauge(size_t i, int buckets_number, double buckets_size) {
    double gauge;
    if(i%3 == 0) {
        gauge = rand() % (buckets_number * (int) buckets_size + (int) buckets_size) + 1;
    }
    else if(i%3 == 1) {
        gauge = rand() % (int) (buckets_number * 3 * pow(2, buckets_number)) + 1;
    }
    else {
        gauge = rand() % (buckets_number * 100) + 1;
    }
    return gauge;
}

int main(int argc, char **argv) {
    int iterations = 20000000;
    double gauge = 0;
    if (argc < 2) {
        printf("No bucket number found.\n");
        exit(EXIT_FAILURE);
    }

    int buckets_number = atoi(argv[1]);
    double buckets_size = 25;
    printf("%d, ", buckets_number);

    double *custom = malloc(sizeof(double) * buckets_number - 1); 
    custom[0] = rand() % (100 + 1);
    for (size_t i = 1; i < buckets_number - 1; i++) {
        custom[i] = custom[i - 1] + rand() % 100 + 1;
    }

    const int dist_number = 3;
    distribution_t *dists[dist_number];
    dists[0] = distribution_new_linear(buckets_number, buckets_size);
    dists[1] = distribution_new_exponential(buckets_number, 3, 2);
    dists[2] = distribution_new_custom(buckets_number - 1, custom);

    struct timespec start_update;
    clock_gettime(CLOCK_MONOTONIC, &start_update);
    for(size_t i = 0; i < iterations; i++) {
        gauge = calculate_gauge(i, buckets_number, buckets_size);
        distribution_update(dists[i%dist_number], gauge);
    }
    struct timespec end_update;
    clock_gettime(CLOCK_MONOTONIC, &end_update);
    double seconds_elapsed_update = end_update.tv_sec - start_update.tv_sec + 1e-9 * (end_update.tv_nsec - start_update.tv_nsec);
    printf("%f, ", seconds_elapsed_update);

    struct timespec start_percentile;
    clock_gettime(CLOCK_MONOTONIC, &start_percentile);
    for(size_t i = 0; i < iterations; i++) {
        distribution_percentile(dists[i%dist_number], rand() % (100 + 1));
    }
    struct timespec end_percentile;
    clock_gettime(CLOCK_MONOTONIC, &end_percentile);
    double seconds_elapsed_percentile = end_percentile.tv_sec - start_percentile.tv_sec + 1e-9 * (end_percentile.tv_nsec - start_percentile.tv_nsec);
    printf("%f, ", seconds_elapsed_percentile);

    struct timespec start_mixed;
    clock_gettime(CLOCK_MONOTONIC, &start_mixed);
    for(size_t i = 0; i < iterations; i++) {
        gauge = calculate_gauge(i, buckets_number, buckets_size);
        i%10 == 0 ? distribution_percentile(dists[i%dist_number], rand() % (100 + 1)) : distribution_update(dists[i%dist_number], gauge);
    }
    struct timespec end_mixed;
    clock_gettime(CLOCK_MONOTONIC, &end_mixed);
    double seconds_elapsed_mixed = end_mixed.tv_sec - start_mixed.tv_sec + 1e-9 * (end_mixed.tv_nsec - start_mixed.tv_nsec);
    printf("%f\n", seconds_elapsed_mixed);
    return 0;
}