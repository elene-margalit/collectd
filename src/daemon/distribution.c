/**
 * collectd - src/daemon/metric.c
 * Copyright (C) 2019-2020  Google LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Author:
 *   Elene Margalitadze <elene.margalit at gmail.com>
 **/

#include <math.h>
#include "collectd.h"
#include "distribution.h"

struct bucket_s{
	uint64_t bucket_counter;
	double min_boundary;
	double max_boundary;
};

struct distribution_s {
     bucket_t *buckets;
     size_t num_buckets;
     uint64_t total_scalar_count;  //count of all registered scalar metrics
     double raw_data_sum;        //sum of all registered raw scalar metrics
};

//private bucket constructor
static bucket_t initialize_bucket(double min_boundary, double max_boundary) {
    bucket_t new_bucket = {
        .bucket_counter = 0,        
        .min_boundary = min_boundary,
        .max_boundary = max_boundary,
    };
    return new_bucket;
} 

//create a new distribution_t with equally sized buckets
distribution_t * distribution_new_linear(size_t num_buckets, double size) {
    if((num_buckets == 0) || (size <= 0)) {
        errno = EINVAL;
        return NULL;
    }

    distribution_t *new_distribution = calloc(1, sizeof(distribution_t));
    if(new_distribution == NULL) {
        return NULL;
    }

    new_distribution->buckets = calloc(num_buckets, sizeof(bucket_t));
    if(new_distribution->buckets == NULL) {
        return NULL;
    }

   for(size_t i = 0; i < num_buckets; i++) {
        if(i < num_buckets - 1) {
            new_distribution->buckets[i] = initialize_bucket(i * size, i*size + size - 1);
        }
        else {
            new_distribution->buckets[i] = initialize_bucket(i * size, INFINITY);
        }
    }
    
    new_distribution->num_buckets = num_buckets;
    new_distribution->total_scalar_count = 0;
    new_distribution->raw_data_sum = 0;
    return new_distribution;
}

//create a new distribution_t with exponentially sized buckets
distribution_t * distribution_new_exponential(size_t num_buckets, double initial_size, double factor) {
    if((num_buckets == 0) || (initial_size <= 0) || (factor < 1)) {
        errno = EINVAL;
        return NULL;
    }
    
    distribution_t *new_distribution = calloc(1, sizeof(distribution_t));
    if(new_distribution == NULL) {
        return NULL;
    }

    new_distribution->buckets = calloc(num_buckets, sizeof(bucket_t));
    if(new_distribution->buckets == NULL) {
        return NULL;
    }

    double previous_bucket_size;
    double new_max;

    for(size_t i = 0; i < num_buckets; i++){
        if(i == 0) {
            new_distribution->buckets[i] = initialize_bucket(0, initial_size - 1);
        }
        else if(i < num_buckets - 1) {
            previous_bucket_size = new_distribution->buckets[i - 1].max_boundary - new_distribution->buckets[i - 1].min_boundary + 1;
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
    return new_distribution;
}

//create a new distribution_t with custom sized buckets, sizes provided in the custom_buckets_sizes array
distribution_t * distribution_new_custom(size_t num_buckets, double *custom_buckets_sizes, size_t arr_size) {
    
    //custom_buckets_arr_size must be num_buckets - 1 so that there is one bucket left for Infinity
    if((num_buckets == 0) || (custom_buckets_sizes == NULL) || (arr_size != num_buckets - 1)) {
        errno = EINVAL;
        return NULL;
    }

    for(size_t i = 0; i < arr_size; i++) {
        if(custom_buckets_sizes[i] <= 0) {
            errno = EINVAL;
            return NULL;
        }
    }

    distribution_t *new_distribution = calloc(1, sizeof(distribution_t));
    if(new_distribution == NULL) {
        return NULL;
    }

    new_distribution->buckets = calloc(num_buckets, sizeof(bucket_t));
    if(new_distribution->buckets == NULL) {
        return NULL;
    }

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
    return new_distribution;
}

//update distributin_t with new incoming scalar metric, increment according bucket
void distribution_update(distribution_t *dist, double gauge) {
    for(size_t i = 0; i < dist->num_buckets; i++) {
        if(gauge >= dist->buckets[i].min_boundary && gauge <= dist->buckets[i].max_boundary) {
            dist->buckets[i].bucket_counter++;
        }
    }

    dist->total_scalar_count++;
    dist->raw_data_sum += gauge;
}

//calculate average value of registered raw scalar metrics
double distribution_average(distribution_t *dist) {
    return dist->raw_data_sum / dist->total_scalar_count;
}

//calculate percentile, returns max_boundary of according bucket, for example 40% took less or equal to 3ms
int distribution_percentile(distribution_t *dist, double percent) {
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

//free memory
void distribution_destroy(distribution_t *dist) {
    if(dist == NULL){
        return;
    }
    free(dist->buckets);
    free(dist);
}


