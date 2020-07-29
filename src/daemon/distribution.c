/**
 * collectd - src/daemon/distribution.c
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
#include "distribution.h"
#include <math.h>

struct bucket_s {
  uint64_t bucket_counter;
  double min_boundary;
  double max_boundary;
};

struct distribution_s {
  bucket_t *buckets;
  size_t num_buckets;
  uint64_t total_scalar_count; // count of all registered scalar metrics
  double raw_data_sum;         // sum of all registered raw scalar metrics
};

/*Private bucket constructor, min_boundary is inclusive, max_boundary is
exclusive because the max_boundary Infinity is exclusive and we want the other
max_boundaries to be consistent with that.*/
static bucket_t initialize_bucket(double min_boundary, double max_boundary) {
  bucket_t new_bucket = {
      .bucket_counter = 0,
      .min_boundary = min_boundary,
      .max_boundary = max_boundary,
  };
  return new_bucket;
}

distribution_t *distribution_new_linear(size_t num_buckets, double size) {
  if ((num_buckets == 0) || (size <= 0)) {
    errno = EINVAL;
    return NULL;
  }

  distribution_t *new_distribution = calloc(1, sizeof(distribution_t));
  bucket_t *buckets = calloc(num_buckets, sizeof(bucket_t));

  if ((new_distribution == NULL) || (buckets == NULL)) {
    free(new_distribution);
    free(buckets);
    return NULL;
  }
  new_distribution->buckets = buckets;

  for (size_t i = 0; i < num_buckets; i++) {
    if (i < num_buckets - 1) {
      new_distribution->buckets[i] =
          initialize_bucket(i * size, i * size + size);
    } else {
      new_distribution->buckets[i] = initialize_bucket(i * size, INFINITY);
    }
  }

  new_distribution->num_buckets = num_buckets;
  new_distribution->total_scalar_count = 0;
  new_distribution->raw_data_sum = 0;
  return new_distribution;
}

distribution_t *distribution_new_exponential(size_t num_buckets,
                                             double base,
                                             double factor) {
  if ((num_buckets == 0) || (base <= 0) || (factor <= 1)) {
    errno = EINVAL;
    return NULL;
  }

  distribution_t *new_distribution = calloc(1, sizeof(distribution_t));
  bucket_t *buckets = calloc(num_buckets, sizeof(bucket_t));

  if ((new_distribution == NULL) || (buckets == NULL)) {
    free(new_distribution);
    free(buckets);
    return NULL;
  }
  new_distribution->buckets = buckets;

  double previous_bucket_size;
  double new_max;

  for (size_t i = 0; i < num_buckets; i++) {
    if (i == 0) {
      new_distribution->buckets[i] = initialize_bucket(0, base);
    } else if (i < num_buckets - 1) {
      /*previous_bucket_size = new_distribution->buckets[i - 1].max_boundary -
                             new_distribution->buckets[i - 1].min_boundary + 1;
      new_max = new_distribution->buckets[i - 1].max_boundary +
                previous_bucket_size * factor;*/
      new_distribution->buckets[i] = initialize_bucket(
          new_distribution->buckets[i - 1].max_boundary, base * pow(factor, i));
    } else {
      new_distribution->buckets[i] = initialize_bucket(
          new_distribution->buckets[i - 1].max_boundary, INFINITY);
    }
  }

  new_distribution->num_buckets = num_buckets;
  new_distribution->total_scalar_count = 0;
  new_distribution->raw_data_sum = 0;
  return new_distribution;
}

distribution_t *distribution_new_custom(size_t num_bounds,
                                        double *custom_max_boundaries) {

  if ((num_bounds == 0) || (custom_max_boundaries == NULL)) {
    errno = EINVAL;
    return NULL;
  }

  for (size_t i = 1; i < num_bounds; i++) {
    if (custom_max_boundaries[i] <= custom_max_boundaries[i - 1]) {
      errno = EINVAL;
      return NULL;
    }
  }

  distribution_t *new_distribution = calloc(1, sizeof(distribution_t));
  bucket_t *buckets =
      calloc(num_bounds + 1, sizeof(bucket_t)); //+1 for infinity bucket

  if ((new_distribution == NULL) || (buckets == NULL)) {
    free(new_distribution);
    free(buckets);
    return NULL;
  }
  new_distribution->buckets = buckets;

  for (size_t i = 0; i < num_bounds + 1; i++) {
    if (i == 0) {
      new_distribution->buckets[i] =
          initialize_bucket(0, custom_max_boundaries[0]);
    } else if (i < num_bounds) {
      new_distribution->buckets[i] =
          initialize_bucket(new_distribution->buckets[i - 1].max_boundary,
                            custom_max_boundaries[i]);
    } else {
      new_distribution->buckets[i] = initialize_bucket(
          new_distribution->buckets[i - 1].max_boundary, INFINITY);
    }
  }

  new_distribution->num_buckets =
      num_bounds + 1; // plus one for infinity bucket
  new_distribution->total_scalar_count = 0;
  new_distribution->raw_data_sum = 0;
  return new_distribution;
}

static size_t binary_search(distribution_t *dist, size_t left, size_t right,
                            double gauge) {
  if (left > right) {
    return -1;
  }

  size_t mid = left + (right - left) / 2;
  if (gauge >= dist->buckets[mid].min_boundary &&
      gauge < dist->buckets[mid].max_boundary) {
    return mid;
  }

  if (gauge < dist->buckets[mid].min_boundary) {
    return binary_search(dist, left, mid, gauge);
  }

  return binary_search(dist, mid, right, gauge);
}

void distribution_update(distribution_t *dist, double gauge) {

  size_t left = 0;
  size_t right = dist->num_buckets - 1;
  size_t index = binary_search(dist, left, right, gauge);
  /*Linear update
  for(size_t i = 0; i < dist->num_buckets; i++) {
      if(gauge >= dist->buckets[i].min_boundary && gauge <=
  dist->buckets[i].max_boundary) { dist->buckets[i].bucket_counter++;
      }
  }*/
  dist->buckets[index].bucket_counter++;
  dist->total_scalar_count++;
  dist->raw_data_sum += gauge;
}

double distribution_average(distribution_t *dist) {
  return dist->raw_data_sum / dist->total_scalar_count;
}

double distribution_percentile(distribution_t *dist, double percent) {
  if ((percent < 0) || (percent > 100)) {
    errno = EINVAL;
    return NAN;
  }

  int sum = 0;
  double bound = 0;
  double target_amount = (percent * 100) / dist->num_buckets;
  for (size_t i = 0; i < dist->num_buckets; i++) {
    sum += dist->buckets[i].bucket_counter;
    if ((double)sum >= target_amount) {
      bound = dist->buckets[i].max_boundary;
      break;
    }
  }
  return bound;
}

distribution_t *distribution_clone(distribution_t *dist) {
  distribution_t *new_distribution = calloc(1, sizeof(distribution_t));
  bucket_t *buckets = calloc(dist->num_buckets, sizeof(bucket_t));

  if ((new_distribution == NULL) || (buckets == NULL)) {
    free(new_distribution);
    free(buckets);
    return NULL;
  }
  new_distribution->buckets = buckets;
  new_distribution->num_buckets = dist->num_buckets;

  for (size_t i = 0; i < new_distribution->num_buckets; i++) {
    new_distribution->buckets[i] = initialize_bucket(
        dist->buckets[i].min_boundary, dist->buckets[i].max_boundary);
  }

  new_distribution->total_scalar_count = dist->total_scalar_count;
  new_distribution->raw_data_sum = dist->total_scalar_count;
  return new_distribution;
}

void distribution_destroy(distribution_t *dist) {
  if (dist == NULL) {
    return;
  }
  free(dist->buckets);
  free(dist);
}
