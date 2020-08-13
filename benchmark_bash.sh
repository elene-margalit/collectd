#!/bin/bash

for i in {50..5000..50}
do
  echo Calculating benchmark for: $i
  ./distribution_benchmark $i >> results.csv
  echo End calculating benchmark
done