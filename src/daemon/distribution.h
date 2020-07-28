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

#include "collectd.h"

struct bucket_s;
typedef struct bucket_s bucket_t;

struct distribution_s;
typedef struct distribution_s distribution_t;


distribution_t * distribution_new_linear(size_t no_buckets, double size);
distribution_t * distribution_new_exponential(size_t num_buckets, double initial_size, double factor);
distribution_t * distribution_new_custom(size_t num_buckets, double *custom_buckets_sizes, size_t arr_size);

void distribution_update(distribution_t *dist, double gauge);
int distribution_percentile(distribution_t *dist, double percent);
double distribution_average(distribution_t *dist);
void distribution_destroy(distribution_t *d);