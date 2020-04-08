#!/bin/bash

ns=12

list=
for g in program_template program_T4 program_T4xyzw program_T4ls_fma
do
  ./process_sequential -s 8192 -o sample_sequential.nc -r result_sequential.nc --generator-factory count -n $ns --use-GPU --GPU-factory $g
  ncks -A profiling_gpu.nc profiling_process.nc
  fo=profiling_GPU_$g.nc
  mv profiling_process.nc $fo
  list=$list" "$fo
done

#ensemble cat
