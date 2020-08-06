/**
 * collectd - src/daemon/distribution_test.c
 * Copyright (C) 2020       Google LLC
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
 */

#include "../testing.h"
#include "collectd.h"
#include "distribution.c"

static double *initialize_linear_min_bounds(size_t num_buckets, double size) {
  double *arr = malloc(num_buckets * sizeof(double));
  for (size_t i = 0; i < num_buckets; i++) {
    arr[i] = i * size;
  }
  return arr;
}

static double *initialize_linear_max_bounds(size_t num_buckets, double size) {
  double *arr = malloc(num_buckets * sizeof(double));
  for (size_t i = 0; i < num_buckets; i++) {
    arr[i] = i == num_buckets - 1 ? INFINITY : i * size + size;
  }
  return arr;
}

static double *initialize_exponential_min_bounds(size_t num_buckets,
                                                 double factor, double base) {
  double *arr = malloc(num_buckets * sizeof(double));
  for (size_t i = 0; i < num_buckets; i++) {
    arr[i] = i == 0 ? 0 : factor * pow(base, i - 1);
  }
  return arr;
}

static double *initialize_exponential_max_bounds(size_t num_buckets,
                                                 double factor, double base) {
  double *arr = malloc(num_buckets * sizeof(double));
  for (size_t i = 0; i < num_buckets; i++) {
    arr[i] = i == num_buckets - 1 ? INFINITY : factor * pow(base, i);
  }
  return arr;
}

static double *initialize_custom_min_bounds(size_t num_buckets,
                                            double *custom_max_boundaries) {
  double *arr = malloc(num_buckets * sizeof(double));
  for (size_t i = 0; i < num_buckets; i++) {
    arr[i] = i == 0 ? 0 : custom_max_boundaries[i - 1];
  }
  return arr;
}

static double *initialize_custom_max_bounds(size_t num_buckets,
                                            double *custom_max_boundaries) {
  double *arr = malloc(num_buckets * sizeof(double));
  for (size_t i = 0; i < num_buckets; i++) {
    arr[i] = i == num_buckets - 1 ? INFINITY : custom_max_boundaries[i];
  }
  return arr;
}

DEF_TEST(distribution_new_linear) {
  struct {
    size_t num_buckets;
    double size;
    int want_err;
    size_t want_get_num_buckets;
    uint64_t want_get_total_scalar_count;
    double want_get_raw_data_sum;
    double *want_get_min_bounds;
    double *want_get_max_bounds;
  } cases[] = {
      {
          .num_buckets = 10,
          .size = 5,
          .want_get_num_buckets = 10,
          .want_get_total_scalar_count = 0,
          .want_get_raw_data_sum = 0.0,
          .want_get_min_bounds = initialize_linear_min_bounds(10, 5),
          .want_get_max_bounds = initialize_linear_max_bounds(10, 5),
      },
      {
          .num_buckets = 0,
          .size = 0,
          .want_err = EINVAL,
      },
      {
          .num_buckets = 240,
          .size = 20,
          .want_get_num_buckets = 240,
          .want_get_total_scalar_count = 0,
          .want_get_raw_data_sum = 0.0,
          .want_get_min_bounds = initialize_linear_min_bounds(240, 20),
          .want_get_max_bounds = initialize_linear_max_bounds(240, 20),
      },
      {
          .num_buckets = 350,
          .size = 15,
          .want_get_num_buckets = 350,
          .want_get_total_scalar_count = 0,
          .want_get_raw_data_sum = 0.0,
          .want_get_min_bounds = initialize_linear_min_bounds(350, 15),
          .want_get_max_bounds = initialize_linear_max_bounds(350, 15),
      },
      {
          .num_buckets = 500,
          .size = 25,
          .want_get_num_buckets = 500,
          .want_get_total_scalar_count = 0,
          .want_get_raw_data_sum = 0.0,
          .want_get_min_bounds = initialize_linear_min_bounds(500, 25),
          .want_get_max_bounds = initialize_linear_max_bounds(500, 25),
      },
  };

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); i++) {
    printf("## Case %zu: \n", i);

    if (cases[i].want_err != 0) {
      EXPECT_EQ_PTR(
          NULL, distribution_new_linear(cases[i].num_buckets, cases[i].size));
      EXPECT_EQ_INT(cases[i].want_err, errno);
      continue;
    }

    distribution_t *dist;
    CHECK_NOT_NULL(
        dist = distribution_new_linear(cases[i].num_buckets, cases[i].size));
    EXPECT_EQ_INT(cases[i].want_get_num_buckets, dist->num_buckets);
    EXPECT_EQ_UINT64(cases[i].want_get_total_scalar_count,
                     dist->total_scalar_count);
    EXPECT_EQ_DOUBLE(cases[i].want_get_raw_data_sum, dist->raw_data_sum);

    for (size_t j = 0; j < dist->num_buckets; j++) {
      EXPECT_EQ_DOUBLE(cases[i].want_get_max_bounds[j],
                       dist->buckets[j].max_boundary);
    }

    for (size_t j = 0; j < dist->num_buckets; j++) {
      EXPECT_EQ_DOUBLE(cases[i].want_get_min_bounds[j],
                       dist->buckets[j].min_boundary);
    }
    distribution_destroy(dist);
    free(cases[i].want_get_max_bounds);
    free(cases[i].want_get_min_bounds);
  }
  return 0;
}

DEF_TEST(distribution_new_exponential) {
  struct {
    size_t num_buckets;
    double start;
    double factor;
    int want_err;
    size_t want_get_num_buckets;
    uint64_t want_get_total_scalar_count;
    double want_get_raw_data_sum;
    double *want_get_min_bounds;
    double *want_get_max_bounds;
  } cases[] = {
      {
          .num_buckets = 10,
          .start = 2,
          .factor = 3,
          .want_get_num_buckets = 10,
          .want_get_total_scalar_count = 0,
          .want_get_raw_data_sum = 0.0,
          .want_get_min_bounds = initialize_exponential_min_bounds(10, 2, 3),
          .want_get_max_bounds = initialize_exponential_max_bounds(10, 2, 3),
      },
      {
          .num_buckets = 0, // In this test case num_buckets, start and factor
                            // are all invalid
          .start = 1,
          .factor = 0,
          .want_err = EINVAL,
      },
      {
          .num_buckets = 0, // num_buckets are invalid
          .start = 2,
          .factor = 3,
          .want_err = EINVAL,
      },
      {
          .num_buckets = 80,
          .start = 0, // start is invalid
          .factor = 3,
          .want_err = EINVAL,
      },
      {
          .num_buckets = 80,
          .start = 2,
          .factor = 1, // factor is invalid
          .want_err = EINVAL,
      },
      {
          .num_buckets = 20,
          .start = 5,
          .factor = 2,
          .want_get_num_buckets = 20,
          .want_get_total_scalar_count = 0,
          .want_get_raw_data_sum = 0.0,
          .want_get_min_bounds = initialize_exponential_min_bounds(20, 5, 2),
          .want_get_max_bounds = initialize_exponential_max_bounds(20, 5, 2),
      },
      {
          .num_buckets = 30,
          .start = 4,
          .factor = 1.5,
          .want_get_num_buckets = 30,
          .want_get_total_scalar_count = 0,
          .want_get_raw_data_sum = 0.0,
          .want_get_min_bounds = initialize_exponential_min_bounds(30, 4, 1.5),
          .want_get_max_bounds = initialize_exponential_max_bounds(30, 4, 1.5),
      },
  };

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); i++) {
    printf("## Case %zu: \n", i);

    if (cases[i].want_err != 0) {
      EXPECT_EQ_PTR(NULL, distribution_new_exponential(cases[i].num_buckets,
                                                       cases[i].start,
                                                       cases[i].factor));
      EXPECT_EQ_INT(cases[i].want_err, errno);
      continue;
    }

    distribution_t *dist;
    CHECK_NOT_NULL(dist = distribution_new_exponential(
                       cases[i].num_buckets, cases[i].start, cases[i].factor));
    EXPECT_EQ_INT(cases[i].want_get_num_buckets, dist->num_buckets);
    EXPECT_EQ_UINT64(cases[i].want_get_total_scalar_count,
                     dist->total_scalar_count);
    EXPECT_EQ_DOUBLE(cases[i].want_get_raw_data_sum, dist->raw_data_sum);

    for (size_t j = 0; j < dist->num_buckets; j++) {
      EXPECT_EQ_DOUBLE(cases[i].want_get_max_bounds[j],
                       dist->buckets[j].max_boundary);
    }

    for (size_t j = 0; j < dist->num_buckets; j++) {
      EXPECT_EQ_DOUBLE(cases[i].want_get_min_bounds[j],
                       dist->buckets[j].min_boundary);
    }
    distribution_destroy(dist);
    free(cases[i].want_get_max_bounds);
    free(cases[i].want_get_min_bounds);
  }
  return 0;
}

DEF_TEST(distribution_new_custom) {
  struct {
    size_t num_bounds;
    double *custom_max_boundaries;
    int want_err;
    size_t want_get_num_buckets;
    uint64_t want_get_total_scalar_count;
    double want_get_raw_data_sum;
    double *want_get_min_bounds;
    double *want_get_max_bounds;
  } cases[] = {
      {
          .num_bounds = 5,
          .custom_max_boundaries = (double[]){5, 10, 20, 30, 50},
          .want_get_num_buckets = 6,
          .want_get_total_scalar_count = 0,
          .want_get_raw_data_sum = 0.0,
          .want_get_min_bounds =
              initialize_custom_min_bounds(6, (double[]){5, 10, 20, 30, 50}),
          .want_get_max_bounds =
              initialize_custom_max_bounds(6, (double[]){5, 10, 20, 30, 50}),
      },
      {
          .num_bounds = 15,
          .custom_max_boundaries =
              (double[]){8, 20, 32, 40, 50, 80, 90, 100, 120, 130, 150, 160,
                         180, 200, 220},
          .want_get_num_buckets = 16,
          .want_get_total_scalar_count = 0,
          .want_get_raw_data_sum = 0.0,
          .want_get_min_bounds = initialize_custom_min_bounds(
              16, (double[]){8, 20, 32, 40, 50, 80, 90, 100, 120, 130, 150, 160,
                             180, 200, 220}),
          .want_get_max_bounds = initialize_custom_max_bounds(
              16, (double[]){8, 20, 32, 40, 50, 80, 90, 100, 120, 130, 150, 160,
                             180, 200, 220}),
      },
      {
          .num_bounds = 0,
          .custom_max_boundaries =
              (double[]){5, 10, 20, 30, 50, 80, 90, 100, 120, 130, 150, 160,
                         180, 200, 220},
          .want_err = EINVAL,
      },
      {
          .num_bounds = 15,
          .custom_max_boundaries =
              (double[]){5, 10, 20, 30, 50, 80, 90, 100, 120, 130, 150, 160,
                         180, 200, INFINITY},
          .want_err = EINVAL,
      },
      {
          .num_bounds = 15,
          .custom_max_boundaries = NULL,
          .want_err = EINVAL,
      },
  };

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); i++) {
    printf("## Case %zu: \n", i);

    if (cases[i].want_err != 0) {
      EXPECT_EQ_PTR(NULL,
                    distribution_new_custom(cases[i].num_bounds,
                                            cases[i].custom_max_boundaries));
      EXPECT_EQ_INT(cases[i].want_err, errno);
      continue;
    }

    distribution_t *dist;
    CHECK_NOT_NULL(dist = distribution_new_custom(
                       cases[i].num_bounds, cases[i].custom_max_boundaries));
    EXPECT_EQ_INT(cases[i].want_get_num_buckets, dist->num_buckets);
    EXPECT_EQ_UINT64(cases[i].want_get_total_scalar_count,
                     dist->total_scalar_count);
    EXPECT_EQ_DOUBLE(cases[i].want_get_raw_data_sum, dist->raw_data_sum);

    for (size_t j = 0; j < dist->num_buckets; j++) {
      EXPECT_EQ_DOUBLE(cases[i].want_get_max_bounds[j],
                       dist->buckets[j].max_boundary);
    }

    for (size_t j = 0; j < dist->num_buckets; j++) {
      EXPECT_EQ_DOUBLE(cases[i].want_get_min_bounds[j],
                       dist->buckets[j].min_boundary);
    }
    distribution_destroy(dist);
    free(cases[i].want_get_max_bounds);
    free(cases[i].want_get_min_bounds);
  }

  return 0;
}

DEF_TEST(distribution_update) {
  struct {
    size_t num_buckets;
    double size;
    int want_err;
    size_t want_get_num_buckets;
    uint64_t want_get_total_scalar_count;
    double want_get_raw_data_sum;
    double gauge;
    size_t want_index;
  } cases[] = {
      {
          .num_buckets = 20,
          .size = 5,
          .want_get_num_buckets = 20,
          .want_get_total_scalar_count = 5,
          .want_get_raw_data_sum = 27.5,
          .gauge = 5.5,
          .want_index = 1,
      },
      {
          .num_buckets = 20,
          .size = 5,
          .want_get_num_buckets = 20,
          .want_get_total_scalar_count = 5,
          .want_get_raw_data_sum = 1500,
          .gauge = 300, // Test if infinity bucket is updated
          .want_index = 19,
      },
      {
          .num_buckets = 10,
          .size = 5,
          .want_get_num_buckets = 10,
          .want_get_total_scalar_count = 5,
          .want_get_raw_data_sum = 57.5,
          .gauge = 11.5,
          .want_index = 2,
      },
      {
          .num_buckets = 240,
          .size = 20,
          .want_get_num_buckets = 240,
          .want_get_total_scalar_count = 5,
          .want_get_raw_data_sum = 0.0,
          .gauge = -1.0,
          .want_err = EINVAL,
      },
  };

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); i++) {

    distribution_t *dist;
    CHECK_NOT_NULL(
        dist = distribution_new_linear(cases[i].num_buckets, cases[i].size));

    if (cases[i].want_err != 0) {
      EXPECT_EQ_INT(cases[i].want_err, errno);
      EXPECT_EQ_INT(-1, distribution_update(dist, cases[i].gauge));
      continue;
    }

    if (cases[i].want_index == -1) {
      continue;
    }

    for (size_t j = 0; j < 5; j++) {
      distribution_update(dist, cases[i].gauge);
    }
    EXPECT_EQ_INT(5, dist->buckets[cases[i].want_index].bucket_counter);
    EXPECT_EQ_INT(cases[i].want_get_num_buckets, dist->num_buckets);
    EXPECT_EQ_UINT64(cases[i].want_get_total_scalar_count,
                     dist->total_scalar_count);
    EXPECT_EQ_DOUBLE(cases[i].want_get_raw_data_sum, dist->raw_data_sum);
    distribution_destroy(dist);
  }
  return 0;
}

DEF_TEST(distribution_percentile) {
  struct {
    size_t num_buckets;
    double size;
    uint64_t total_scalar_count;
    uint64_t *bucket_counters;
    double percent;
    double want_get_result_bound;
    int want_err;
  } cases[] = {
      {
          .num_buckets = 20,
          .size = 15,
          .total_scalar_count = 30,
          .bucket_counters = (uint64_t[]){5, 10, 4, 3, 2, 2, 3, 0, 0, 0,
                                          0, 0,  0, 0, 0, 0, 0, 0, 0, 0},
          .percent = 40,
          .want_get_result_bound = 30,
      },
      {
          .num_buckets = 25,
          .size = 30,
          .total_scalar_count = 70,
          .bucket_counters =
              (uint64_t[]){19, 20, 5, 14, 8, 7, 2, 2, 1, 1, 0, 0, 0,
                           0,  0,  0, 0,  0, 0, 0, 0, 0, 0, 0, 0},
          .percent = 85,
          .want_get_result_bound = 150,
      },
      {
          .num_buckets = 15,
          .size = 20,
          .total_scalar_count = 55,
          .bucket_counters =
              (uint64_t[]){0, 0, 2, 3, 5, 8, 10, 12, 14, 1, 0, 0, 0, 0, 0},
          .percent = 25,
          .want_get_result_bound = 120,
      },
      {
          .num_buckets = 15,
          .size = 20,
          .percent = 100.5,
          .want_err = EINVAL,
      },
  };

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); i++) {
    printf("## Case %zu: \n", i);

    distribution_t *dist =
        distribution_new_linear(cases[i].num_buckets, cases[i].size);

    if (cases[i].want_err != 0) {
      EXPECT_EQ_DOUBLE(NAN, distribution_percentile(dist, cases[i].percent));
      EXPECT_EQ_INT(cases[i].want_err, errno);
      continue;
    }

    dist->total_scalar_count = cases[i].total_scalar_count;
    for (size_t j = 0; j < dist->num_buckets; j++) {
      dist->buckets[j].bucket_counter = cases[i].bucket_counters[j];
    }

    EXPECT_EQ_DOUBLE(cases[i].want_get_result_bound,
                     distribution_percentile(dist, cases[i].percent));
    distribution_destroy(dist);
  }
  return 0;
}

DEF_TEST(distribution_average) {
  struct {
    size_t num_buckets;
    double size;
    double *gauges;
    uint64_t total_scalar_count;
    double raw_data_sum;
    int want_err;
    double want_get_average;

  } cases[] = {
      {
          .num_buckets = 20,
          .size = 15,
          .gauges = (double[]){20.25, 19.3, 5.5,   89,  75.5,  60.8, 40.2,
                               38,    31,   130.8, 111, 102.4, 95.5, 60,
                               98,    45,   53.3,  9,   12,    140.5},
          .total_scalar_count = 20,
          .raw_data_sum = 1237.05,
          .want_get_average = 61.8525,
      },
      {
          .num_buckets = 0, // invalid number of buckets
          .size = 25,
          .want_err = EINVAL,
      },
      {
          .num_buckets = 10,
          .size = 25,
          .want_get_average = NAN,
      },
      {
          .num_buckets = 25,
          .size = 10,
          .gauges = (double[]){2,     4.3,   78,   19,  55.5, 20.3, 32.9,
                               39,    200.8, 130,  101, 52.4, 95.5, 40,
                               108.9, 36,    53.3, 98,  12,   300},
          .total_scalar_count = 20,
          .raw_data_sum = 1478.9,
          .want_get_average = 73.945,
      },
  };

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); i++) {

    if (cases[i].want_err != 0) {
      distribution_t *dist =
          distribution_new_linear(cases[i].num_buckets, cases[i].size);
      EXPECT_EQ_DOUBLE(NAN, distribution_average(dist));
      EXPECT_EQ_INT(cases[i].want_err, errno);
      continue;
    }

    distribution_t *dist =
        distribution_new_linear(cases[i].num_buckets, cases[i].size);

    if (cases[i].gauges != NULL) {
      for (size_t j = 0; j < cases[i].total_scalar_count; j++) {
        distribution_update(dist, cases[i].gauges[j]);
      }
    }

    EXPECT_EQ_DOUBLE(cases[i].want_get_average, distribution_average(dist));
    distribution_destroy(dist);
  }
  return 0;
}

DEF_TEST(distribution_clone) {
  struct {
    size_t num_buckets;
    double size;
    double *gauges;
    uint64_t total_scalar_count;
    int want_err;
  } cases[] = {
      {
          .num_buckets = 50,
          .size = 45,
          .gauges = (double[]){20.25, 19.3, 5.5,   89,  75.5,  60.8, 40.2,
                               38,    31,   130.8, 111, 102.4, 95.5, 60,
                               98,    45,   53.3,  9,   12,    140.5},
          .total_scalar_count = 20,
      },
      {
          .num_buckets = 0, // invalid bucket number
          .size = 45,
          .want_err = EINVAL,
      },
  };

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); i++) {

    if (cases[i].want_err != 0) {
      distribution_t *dist =
          distribution_new_linear(cases[i].num_buckets, cases[i].size);
      distribution_t *clone = distribution_clone(dist);
      EXPECT_EQ_PTR(dist, clone);
      continue;
    }
    distribution_t *dist =
        distribution_new_linear(cases[i].num_buckets, cases[i].size);

    for (size_t j = 0; j < cases[i].total_scalar_count; j++) {
      distribution_update(dist, cases[i].gauges[j]);
    }
    distribution_t *clone = distribution_clone(dist);
    EXPECT_EQ_INT(dist->num_buckets, clone->num_buckets);
    EXPECT_EQ_UINT64(dist->total_scalar_count, clone->total_scalar_count);
    EXPECT_EQ_DOUBLE(dist->raw_data_sum, clone->raw_data_sum);

    for (size_t j = 0; j < cases[i].num_buckets; j++) {
      EXPECT_EQ_INT(dist->buckets[j].bucket_counter,
                    clone->buckets[j].bucket_counter);
    }
    distribution_destroy(dist);
    distribution_destroy(clone);
  }
  return 0;
}

DEF_TEST(distribution_get_buckets) {
  struct {
    size_t num_buckets;
    double size;
    int want_err;
  } cases[] = {
      {
          .num_buckets = 50,
          .size = 45,
      },
      {
          .num_buckets = 35,
          .size = 20,
      },
  };

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); i++) {

    if (cases[i].want_err != 0) {
      continue;
    }
    distribution_t *dist =
        distribution_new_linear(cases[i].num_buckets, cases[i].size);

    bucket_t *buckets;
    CHECK_NOT_NULL(buckets = distribution_get_buckets(dist));
    EXPECT_EQ_INT(dist->num_buckets, distribution_get_num_buckets(dist));
    for (size_t j = 0; j < cases[i].num_buckets; j++) {
      EXPECT_EQ_INT(dist->buckets[j].bucket_counter, buckets[j].bucket_counter);
    }
    free(buckets);
    distribution_destroy(dist);
  }
  return 0;
}

DEF_TEST(distribution_get_total_scalar_count) {
  struct {
    size_t num_buckets;
    double size;
    uint64_t total_scalar_count;
    double *gauges;
    int want_err;
  } cases[] = {
      {
          .num_buckets = 50,
          .size = 45,
          .total_scalar_count = 20,
          .gauges = (double[]){20.25, 19.3, 5.5,   89,  75.5,  60.8, 40.2,
                               38,    31,   130.8, 111, 102.4, 95.5, 60,
                               98,    45,   53.3,  9,   12,    140.5},
      },
      {
          .num_buckets = 35,
          .size = 20,
          .total_scalar_count = 20,
          .gauges = (double[]){2,     4.3,   78,   19,  55.5, 20.3, 32.9,
                               39,    200.8, 130,  101, 52.4, 95.5, 40,
                               108.9, 36,    53.3, 98,  12,   240.7},
      },
      {
          .num_buckets = 0,
          .size = 45,
          .want_err = EINVAL,
      },
  };

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); i++) {
    distribution_t *dist =
        distribution_new_linear(cases[i].num_buckets, cases[i].size);

    if (cases[i].want_err != 0) {
      EXPECT_EQ_PTR(NULL, distribution_get_buckets(dist));
      EXPECT_EQ_INT(-1, distribution_get_total_scalar_count(dist));
      EXPECT_EQ_INT(cases[i].want_err, errno);
      continue;
    }

    for (size_t j = 0; j < cases[i].total_scalar_count; j++) {
      distribution_update(dist, cases[i].gauges[j]);
    }
    EXPECT_EQ_INT(dist->total_scalar_count,
                  distribution_get_total_scalar_count(dist));
    distribution_destroy(dist);
  }
  return 0;
}

DEF_TEST(distribution_get_raw_data_sum) {
  struct {
    size_t num_buckets;
    double size;
    uint64_t total_scalar_count;
    double raw_data_sum;
    double *gauges;
    int want_err;
  } cases[] = {
      {
          .num_buckets = 50,
          .size = 45,
          .total_scalar_count = 20,
          .raw_data_sum = 1237.05,
          .gauges = (double[]){20.25, 19.3, 5.5,   89,  75.5,  60.8, 40.2,
                               38,    31,   130.8, 111, 102.4, 95.5, 60,
                               98,    45,   53.3,  9,   12,    140.5},
      },
      {
          .num_buckets = 35,
          .size = 20,
          .total_scalar_count = 20,
          .raw_data_sum = 1419.6,
          .gauges = (double[]){2,     4.3,   78,   19,  55.5, 20.3, 32.9,
                               39,    200.8, 130,  101, 52.4, 95.5, 40,
                               108.9, 36,    53.3, 98,  12,   240.7},
      },
      {
          .num_buckets = 0,
          .size = 45,
          .want_err = EINVAL,
      },
  };

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); i++) {

    distribution_t *dist =
        distribution_new_linear(cases[i].num_buckets, cases[i].size);

    if (cases[i].want_err != 0) {
      EXPECT_EQ_PTR(NULL, distribution_get_buckets(dist));
      EXPECT_EQ_DOUBLE(NAN, distribution_get_raw_data_sum(dist));
      EXPECT_EQ_INT(cases[i].want_err, errno);
      continue;
    }

    for (size_t j = 0; j < cases[i].total_scalar_count; j++) {
      distribution_update(dist, cases[i].gauges[j]);
    }
    EXPECT_EQ_DOUBLE(dist->raw_data_sum, distribution_get_raw_data_sum(dist));
    distribution_destroy(dist);
  }
  return 0;
}

int main(void) {
  RUN_TEST(distribution_new_linear);
  RUN_TEST(distribution_new_exponential);
  RUN_TEST(distribution_new_custom);
  RUN_TEST(distribution_update);
  RUN_TEST(distribution_average);
  RUN_TEST(distribution_percentile);
  RUN_TEST(distribution_clone);
  RUN_TEST(distribution_get_buckets);
  RUN_TEST(distribution_get_total_scalar_count);
  RUN_TEST(distribution_get_raw_data_sum);
  END_TEST;
}