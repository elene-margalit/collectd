#include "collectd.h"

struct bucket_s;
typedef struct bucket_s bucket_t;

struct distribution_s;
typedef struct distribution_s distribution_t;


distribution_t * distribution_new_linear(size_t no_buckets, double size);
distribution_t * distribution_new_exponential(size_t no_buckets, double initial_size, double factor);
distribution_t * distribution_new_custom(size_t no_buckets, double *custom_buckets_sizes);

void distribution_update(distribution_t *dist, double gauge);
int distribution_percentile(distribution_t *dist, uint8_t percent);
double distribution_average(distribution_t *dist);
void distribution_destroy(distribution_t *d);
