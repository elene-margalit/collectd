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

/*Introduce data structure bucket_t with which we will count the occurrences of
scalar metrics falling into specific ranges*/
typedef struct {
  uint64_t bucket_counter;
  double min_boundary;
  double max_boundary;
} bucket_t;

/*Introduce data structure distribution_t which will provide us with all
neccessary functions and attributes to get insight into the distribution of
collected scalar metrics.*/
struct distribution_s;
typedef struct distribution_s distribution_t;

/*All three constructor functions check if calloc failed and return NULL in that
 * case.*/

/*Constructor function for creating buckets with linearly sized ranges.
Error Handling: if num_buckets == 0 or size is <= 0, errno is set to EINVAL and
NULL is returned.*/
distribution_t *distribution_new_linear(size_t num_buckets, double size);

/*Constructor function for creating buckets with exponentially sized ranges. The
factor defines the size of the first bucket and each subsequent bucket has
the size factor * pow(base, i). Error Handling: if num_buckets == 0
or base is <= 0 or factor <= 1, errno is set to EINVAL and NULL is
returned.*/
distribution_t *distribution_new_exponential(size_t num_buckets, double factor,
                                             double base);

/*Constructor function for creating buckets with custom sized ranges. The array
custom_max_boundaries contains custom max_boundaries for each bucket and the
num_bounds represents the size of this array. The num_buckets attribute is
set to num_bounds + 1 in the function because we have to account for the
bucket with max_boundary +Infinity. Error Handling: if num_boundaries == 0 or
custom_max_boundaries == NULL or the array elements are not increasing,
errno is set to EINVAL and NULL is returned.*/
distribution_t *distribution_new_custom(size_t num_bounds,
                                        double *custom_max_boundaries);

/*Function for updating our distribution_t object when a new scalar metric gauge
is collected using binary search. The corresponding bucket counter is
incremented, as is the total_scalar_count attribute of distribution_t. We also
add the value of gauge to the
attribute raw_data_sum which sums up all collected raw scalar metrics.
Error Handling: If dist == NULL or gauge <= 0, errno is set to EINVAL and -1 is
returned, else return 0*/
int distribution_update(distribution_t *dist, double gauge);

/*Function for calculating the desired percentile. It returns the max_boundary
of the corresponding bucket. For example, if we want to calculate the 40th
percentile, the function will inform us that 40% of the collected scalar metrics
fall under a specific max_boundarary.
Error Handling: if dist == NULL or percent is less than 0 or more than 100,
errno is set to EINVAL and the function returns NAN.*/
double distribution_percentile(distribution_t *dist, double percent);

/*Returns the average of all collected raw scalar metrics.
Error Handling: if dist == NULL, errno is set to EINVAL and
NULL is returned.*/
double distribution_average(distribution_t *dist);

/*Create a clone of the argument distribution_t dist, which will represent a
snapshot for when we want to calculate a percentile or average. If we don't
provide a cloned distribution_t as an argument to the percentile and average
functions, the update function might overwrite the distribution_t object while
calculating the percentile or average and that we would like to avoid.
Error Handling: if dist == NULL, errno is set to EINVAL and
NULL is returned.*/
distribution_t *distribution_clone(distribution_t *dist);

/*Function for freeing the memory of a distribution_t object after we don't need
it anymore.
Error Handling: if dist == NULL, errno is set to EINVAL*/
void distribution_destroy(distribution_t *d);

/*Getter function for getting the buckets.
Error Handling: if dist == NULL, errno is set to EINVAL and
NULL is returned.*/
bucket_t *distribution_get_buckets(distribution_t *dist);

/*Getter function for number of buckets
Error Handling: if dist == NULL, errno is set to EINVAL and
-1 is returned.*/
int distribution_get_num_buckets(distribution_t *dist);

/*Getter function for total count of all registered scalar metrics.
Error Handling: if dist == NULL, errno is set to EINVAL and
-1 is returned.*/
int distribution_get_total_scalar_count(distribution_t *dist);

/*Getter function for sum of all registered raw scalar metrics.
Error Handling: if dist == NULL, errno is set to EINVAL and
NAN is returned.*/
double distribution_get_raw_data_sum(distribution_t *dist);