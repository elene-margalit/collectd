/**
 * collectd - src/daemon/distribution.h
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

#include "collectd.h"

/*Introduce data structure bucket_t with which we will count the occurrences of scalar metrics falling into specific
ranges*/
struct bucket_s;
typedef struct bucket_s bucket_t;

/*Introduce data structure distribution_t which will provide us with all neccessary functions and attributes to get 
insight into the distribution of collected scalar metrics.*/
struct distribution_s;
typedef struct distribution_s distribution_t;

/*Constructor function for creating buckets with linearly sized ranges.*/
distribution_t * distribution_new_linear(size_t num_buckets, double size);

/*Constructor function for creating buckets with exponentially sized ranges. The initial_size defines the size of the
first bucket and each subsequent bucket has the size factor*(size of previous bucket).*/
distribution_t * distribution_new_exponential(size_t num_buckets, double initial_size, double factor);

/*Constructor function for creating buckets with custom sized ranges. The array custom_bucket_sizes contains
custom max_boundaries for each bucket and the arr_size has to be equal to num_buckets - 1 because we need one more
bucket with the max_boundary of infinity.*/
distribution_t * distribution_new_custom(size_t num_buckets, double *custom_bucket_boundaries, size_t arr_size);

/*Function for updating our distribution_t object when a new scalar metric gauge is collected. The corresponding bucket 
counter is incremented, as is the total_scalar_count attribute of distribution_t. We also add the value of gauge to the
attribute raw_data_sum which sums up all collected raw scalar metrics.*/
void distribution_update(distribution_t *dist, double gauge);

/*Function for calculating the desired percentile. It returns the max_boundary of the corresponding bucket. For example, 
if we want to calculate the 40th percentile, the function will inform us that 40% of the collected scalar metrics fall
under a specific max_boundarary.*/
int distribution_percentile(distribution_t *dist, double percent);

/*Returns the average of all collected raw scalar metrics.*/
double distribution_average(distribution_t *dist);

/*Function for freeing the memory of a distribution_t object after we don't need it anymore.*/
void distribution_destroy(distribution_t *d);