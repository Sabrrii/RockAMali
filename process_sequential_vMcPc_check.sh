#!/bin/bash

echo 'parse a few factory entities with data check:'
for f in program lambda closure function function_lambda function_macro program_template program_T4 program_T4xyzw program_T4ls_fma
do
  ./process_sequential -s 4096 -o sample_sequential.nc -r result_sequential.nc --generator-factory count --CPU-factory kernel -n 12 --use-GPU --GPU-factory $f --do-check 2>/dev/null | grep -e 'test:' -e max
done
