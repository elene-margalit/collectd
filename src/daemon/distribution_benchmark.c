#define _POSIX_C_SOURCE 199309L
#include "distribution.h"
#include <stdio.h>
#include <time.h>

const size_t iterations = 1000000;
const int dist_number = 3;

double calculate_gauge(size_t i, int buckets_number, double buckets_size, double factor, double base) {
    double gauge;
    if(i%3 == 0) {
        gauge = rand() % (buckets_number * (int) buckets_size + (int) buckets_size) + 1;
    }
    else if(i%3 == 1) {
        gauge = rand() % (int) (buckets_number * factor * pow(base, buckets_number)) + 1;
    }
    else {
        gauge = rand() % (buckets_number * 100) + 1;
    }
    return gauge;
}

static double * calculate_gauges_arr(double gauges[], size_t iterations) {
    for(size_t i = 0; i < iterations; i++) {
        gauges[i] = (double) (rand() % (int) 1e6);
    }
    return gauges;
}

static double * calculate_percents_arr(double percents[], size_t iterations) {
    for(size_t i = 0; i < iterations; i++) {
        percents[i] =  (double) (rand() % 101);
    }
    return percents;
}

static size_t * calculate_dist_index_arr(size_t indexes[], size_t iterations) {
    for(size_t i = 0; i < iterations; i++) {
        indexes[i] = rand() % 3;
    }
    return indexes;
}

void measure_update(distribution_t *dist, size_t iterations, double *gauges) {
    struct timespec start_update;
    clock_gettime(CLOCK_MONOTONIC, &start_update);
    for(size_t i = 0; i < iterations; i++) {
        distribution_update(dist, gauges[i]);
    }
    struct timespec end_update;
    clock_gettime(CLOCK_MONOTONIC, &end_update);
    double seconds_elapsed_update = end_update.tv_sec - start_update.tv_sec + 1e-9 * (end_update.tv_nsec - start_update.tv_nsec);
    printf("%f, ", seconds_elapsed_update);
}

void measure_percentile(distribution_t *dist, size_t iterations, double *percents) {
    struct timespec start_update;
    clock_gettime(CLOCK_MONOTONIC, &start_update);
    for(size_t i = 0; i < iterations; i++) {
        distribution_percentile(dist, percents[i]);
    }
    struct timespec end_update;
    clock_gettime(CLOCK_MONOTONIC, &end_update);
    double seconds_elapsed_update = end_update.tv_sec - start_update.tv_sec + 1e-9 * (end_update.tv_nsec - start_update.tv_nsec);
    printf("%f, ", seconds_elapsed_update);
}

void measure_mixed(distribution_t *dist, size_t iterations, double *percents, double *gauges) {
    struct timespec start_update;
    clock_gettime(CLOCK_MONOTONIC, &start_update);
    for(size_t i = 0; i < iterations; i++) {
        i%10 == 0 ? distribution_percentile(dist, percents[i]) : distribution_update(dist, gauges[i]);
    }
    struct timespec end_update;
    clock_gettime(CLOCK_MONOTONIC, &end_update);
    double seconds_elapsed_update = end_update.tv_sec - start_update.tv_sec + 1e-9 * (end_update.tv_nsec - start_update.tv_nsec);
    printf("%f, \n", seconds_elapsed_update);
}

void measure_update_all_dists(distribution_t **dists, size_t iterations, double *gauges, size_t *indexes) {
    struct timespec start_update;
    clock_gettime(CLOCK_MONOTONIC, &start_update);
    for(size_t i = 0; i < iterations; i++) {
        distribution_update(dists[indexes[i]], gauges[i]);
    }
    struct timespec end_update;
    clock_gettime(CLOCK_MONOTONIC, &end_update);
    double seconds_elapsed_update = end_update.tv_sec - start_update.tv_sec + 1e-9 * (end_update.tv_nsec - start_update.tv_nsec);
    printf("%f, ", seconds_elapsed_update);
}

void measure_percentile_all_dists(distribution_t **dists, size_t iterations, double *percents, size_t *indexes) {
    struct timespec start_update;
    clock_gettime(CLOCK_MONOTONIC, &start_update);
    for(size_t i = 0; i < iterations; i++) {
        distribution_update(dists[indexes[i]], percents[i]);
    }
    struct timespec end_update;
    clock_gettime(CLOCK_MONOTONIC, &end_update);
    double seconds_elapsed_update = end_update.tv_sec - start_update.tv_sec + 1e-9 * (end_update.tv_nsec - start_update.tv_nsec);
    printf("%f, ", seconds_elapsed_update);
}

void measure_mixed_all_dists(distribution_t **dists, size_t iterations, double *percents, double *gauges, size_t *indexes) {
    struct timespec start_mixed;
    clock_gettime(CLOCK_MONOTONIC, &start_mixed);
    for(size_t i = 0; i < iterations; i++) {
        i%10 == 0 ? distribution_percentile(dists[indexes[i]], percents[i]) : distribution_update(dists[indexes[i]], gauges[i]);
    }
    struct timespec end_mixed;
    clock_gettime(CLOCK_MONOTONIC, &end_mixed);
    double seconds_elapsed_mixed = end_mixed.tv_sec - start_mixed.tv_sec + 1e-9 * (end_mixed.tv_nsec - start_mixed.tv_nsec);
    printf("%f\n", seconds_elapsed_mixed);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("No bucket number found.\n");
        exit(EXIT_FAILURE);
    }

    int buckets_number = atoi(argv[1]);
    double buckets_size = 25;

    double *custom = malloc(sizeof(double) * buckets_number - 1);
    if(custom == NULL) {
        printf("Malloc failed.\n");
        exit(EXIT_FAILURE);
    } 

    custom[0] = rand() % (100 + 1);
    for (size_t i = 1; i < buckets_number - 1; i++) {
        custom[i] = custom[i - 1] + rand() % 100 + 1;
    }

    distribution_t *dists[dist_number];
    dists[0] = distribution_new_linear(buckets_number, buckets_size);
    dists[1] = distribution_new_exponential(buckets_number, 3, 2);
    dists[2] = distribution_new_custom(buckets_number - 1, custom);

    double *gauges = malloc(sizeof(double) * iterations);
    if(gauges == NULL) {
        printf("Malloc failed.\n");
        exit(EXIT_FAILURE);
    } 
    gauges = calculate_gauges_arr(gauges, iterations);

    double *percents = malloc(sizeof(double) * iterations);
    if(percents == NULL) {
        printf("Malloc failed.\n");
        exit(EXIT_FAILURE);
    } 
    percents = calculate_percents_arr(percents, iterations);

    size_t *indexes = malloc(sizeof(size_t) * iterations);
    if(indexes == NULL) {
        printf("Malloc failed.\n");
        exit(EXIT_FAILURE);
    } 
    indexes = calculate_dist_index_arr(indexes, iterations);

    printf("%d, ", buckets_number);
    measure_update(dists[0], iterations, gauges);
    measure_percentile(dists[0], iterations, percents);
    measure_mixed(dists[0], iterations, percents, gauges);

    printf("%d, ", buckets_number);
    measure_update(dists[1], iterations, gauges);
    measure_percentile(dists[1], iterations, percents);
    measure_mixed(dists[1], iterations, percents, gauges);

    printf("%d, ", buckets_number);
    measure_update(dists[2], iterations, gauges);
    measure_percentile(dists[2], iterations, percents);
    measure_mixed(dists[2], iterations, percents, gauges);

    printf("%d, ", buckets_number);
    measure_update_all_dists(dists, iterations, gauges, indexes);
    measure_percentile_all_dists(dists, iterations, gauges, indexes);
    measure_mixed_all_dists(dists, iterations, percents, gauges, indexes);
    return 0;
}