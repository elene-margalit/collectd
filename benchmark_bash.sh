#!/bin/bash

for i in {20..4000..20}
do
  echo Calculating benchmark for: $i
  ./distribution_benchmark $i >> results.csv
  echo End calculating benchmark
done