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
     size_t num_buckets;
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

distribution_t * distribution_new_linear(size_t num_buckets, double size) {
    if(num_buckets <= 0 || size <= 0) {
        errno = EINVAL;
        return NULL;
    }

    distribution_t *new_distribution = calloc(1, sizeof(distribution_t));
    new_distribution->buckets = calloc(num_buckets, sizeof(bucket_t));

   for(size_t i = 0; i < num_buckets; i++) {
        if(i == 0) {
            new_distribution->buckets[i] = initialize_bucket(0, size);
        }
        else if(i < num_buckets - 1) {
            new_distribution->buckets[i] = initialize_bucket(i * size + 1, i*size + size);
        }
        else {
            new_distribution->buckets[i] = initialize_bucket(new_distribution->buckets[i - 1].max_boundary + 1, INFINITY);
        }
    }
    
    new_distribution->num_buckets = num_buckets;
    new_distribution->total_scalar_count = 0;
    new_distribution->raw_data_sum = 0;
}

distribution_t * distribution_new_exponential(size_t num_buckets, double initial_size, double factor) {
    if(num_buckets <= 0 || initial_size <= 0 || factor <= 0) {
        errno = EINVAL;
        return NULL;
    }
    
    distribution_t *new_distribution = calloc(1, sizeof(distribution_t));
    new_distribution->buckets = calloc(num_buckets, sizeof(bucket_t));
    double previous_bucket_size;
    double new_max;

    for(int i = 0; i < num_buckets; i++){
        if(i == 0) {
            new_distribution->buckets[i] = initialize_bucket(0, initial_size);
        }
        else if(i < num_buckets - 1) {
            previous_bucket_size = new_distribution->buckets[i - 1].max_boundary - new_distribution->buckets[i - 1].min_boundary;
            new_max = new_distribution->buckets[i - 1].max_boundary + previous_bucket_size * factor;
            new_distribution->buckets[i] = initialize_bucket(new_distribution->buckets[i - 1].max_boundary + 1, new_max);
        }
        else {
            new_distribution->buckets[i] = initialize_bucket(new_distribution->buckets[i - 1].max_boundary + 1, INFINITY);
        }
    }
    
    new_distribution->num_buckets = num_buckets;
    new_distribution->total_scalar_count = 0;
    new_distribution->raw_data_sum = 0;
}

distribution_t * distribution_new_custom(size_t num_buckets, double *custom_buckets_sizes, size_t custom_buckets_arr_size) {
    //custom_buckets_arr_size must be num_buckets - 1 so that there is one bucket left for Infinity
    if(num_buckets <= 0 || custom_buckets_sizes == NULL || custom_buckets_arr_size != num_buckets - 1) {
        errno = EINVAL;
        return NULL;
    }

    for(size_t i = 0; i < custom_buckets_arr_size; i++) {
        if(custom_buckets_sizes[i] <= 0) {
            errno = EINVAL;
            return NULL;
        }
    }

    distribution_t *new_distribution = calloc(1, sizeof(distribution_t));
    new_distribution->buckets = calloc(num_buckets, sizeof(bucket_t));

    for(size_t i = 0; i < num_buckets; i++) {
        if(i == 0) {
            new_distribution->buckets[i] = initialize_bucket(0, custom_buckets_sizes[0] - 1);
        }
        else if(i < num_buckets - 1) {
            double current_min = new_distribution->buckets[i - 1].max_boundary + 1;
            double current_max = new_distribution->buckets[i - 1].max_boundary + custom_buckets_sizes[i];
            new_distribution->buckets[i] = initialize_bucket(current_min, current_max);
        }
        else {
            new_distribution->buckets[i] = initialize_bucket(new_distribution->buckets[i - 1].max_boundary + 1, INFINITY);
        }
    }
    
    new_distribution->num_buckets = num_buckets;
    new_distribution->total_scalar_count = 0;
    new_distribution->raw_data_sum = 0;
}

void distribution_update(distribution_t *dist, double gauge) {
    for(size_t i = 0; i < dist->num_buckets; i++) {
        if(gauge >= dist->buckets[i].min_boundary && gauge <= dist->buckets[i].max_boundary) {
            dist->buckets[i].bucket_counter++;
        }
    }
}

double distribution_average(distribution_t *dist) {
    return dist->raw_data_sum / dist->total_scalar_count;
}

int distribution_percentile(distribution_t *dist, uint8_t percent) {
    int sum = 0;
    int bound = 0;
    double target_amount = (percent * 100) / dist->num_buckets;
    for(size_t i = 0; i < dist->num_buckets; i++) {
        sum += dist->buckets[i].bucket_counter;
        if(sum >= target_amount) {
            bound = dist->buckets[i].max_boundary;
            break;
        }
    }
    return bound;
}

void distribution_destroy(distribution_t *dist) {
    if(dist == NULL){
        return;
    }
    free(dist->buckets);
    free(dist);
}


