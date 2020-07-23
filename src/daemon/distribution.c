#include <math.h>
#include "collectd.h"
#include "distribution.h"

struct bucket_s{
	size_t bucket_counter;
	double min_boundary;
	double max_boundary;
};

struct distribution_s {
     bucket_t *buckets;
     size_t no_buckets;
     size_t total_scalar_count;
     double raw_data_sum;
};

//bucket constructor
bucket_t initialize_bucket(double min_boundary, double max_boundary) {
    bucket_t new_bucket = {
        .bucket_counter = 0,
        .min_boundary = min_boundary,
        .max_boundary = max_boundary,
    };
    return new_bucket;
} 

//increment bucket_counter of specific bucket
void increment(bucket_t *bucket) { 
    bucket->bucket_counter++;
} 

distribution_t * distribution_new_linear(size_t no_buckets, double size) {
    distribution_t *new_distribution = calloc(1, sizeof(distribution_t));
    new_distribution->buckets = calloc(no_buckets, sizeof(bucket_t));

   for(size_t i = 0; i < no_buckets; i++)
    {
        if(i == 0)
        {
            new_distribution->buckets[i] = initialize_bucket(0, size);
        }
        else if(i < no_buckets - 1)
        {
            new_distribution->buckets[i] = initialize_bucket(i * size + 1, i*size + size);
        }
        else 
        {
            new_distribution->buckets[i] = initialize_bucket(new_distribution->buckets[i - 1].max_boundary + 1, INFINITY);
        }
    }
    
    new_distribution->no_buckets = no_buckets;
    new_distribution->total_scalar_count = 0;
    new_distribution->raw_data_sum = 0;
}

distribution_t * distribution_new_exponential(size_t no_buckets, double initial_size, double factor) {
    distribution_t *new_distribution = calloc(1, sizeof(distribution_t));
    new_distribution->buckets = calloc(no_buckets, sizeof(bucket_t));
    double previous_bucket_size;
    double new_max;

    for(int i = 0; i < no_buckets; i++)
    {
        if(i == 0)
        {
            new_distribution->buckets[i] = initialize_bucket(0, initial_size);
        }
        else if(i < no_buckets - 1)
        {
            previous_bucket_size = new_distribution->buckets[i - 1].max_boundary - new_distribution->buckets[i - 1].min_boundary;
            new_max = new_distribution->buckets[i - 1].max_boundary + previous_bucket_size * factor;
            new_distribution->buckets[i] = initialize_bucket(new_distribution->buckets[i - 1].max_boundary + 1, new_max);
        }
        else
        {
            new_distribution->buckets[i] = initialize_bucket(new_distribution->buckets[i - 1].max_boundary + 1, INFINITY);
        }
        
    }
    
    new_distribution->no_buckets = no_buckets;
    new_distribution->total_scalar_count = 0;
    new_distribution->raw_data_sum = 0;
}

distribution_t * distribution_new_custom(size_t no_buckets, double *custom_buckets_sizes) {
    distribution_t *new_distribution = calloc(1, sizeof(distribution_t));
    new_distribution->buckets = calloc(no_buckets, sizeof(bucket_t));

    for(size_t i = 0; i < no_buckets; i++)
    {
        if(i == 0)
        {
            new_distribution->buckets[i] = initialize_bucket(0, custom_buckets_sizes[0]);
        }
        else if(i < no_buckets - 1)
        {
            new_distribution->buckets[i] = initialize_bucket(new_distribution->buckets[i - 1].max_boundary + 1, custom_buckets_sizes[i]);
        }
        else
        {
            new_distribution->buckets[i] = initialize_bucket(new_distribution->buckets[i - 1].max_boundary + 1, INFINITY);
        }
    }
    
    new_distribution->no_buckets = no_buckets;
    new_distribution->total_scalar_count = 0;
    new_distribution->raw_data_sum = 0;
}

double distribution_average(distribution_t *dist) {
    return dist->raw_data_sum / dist->total_scalar_count;
}

int distribution_percentile(distribution_t *dist, uint8_t percent) {

}

void distribution_destroy(distribution_t *d) {

}


