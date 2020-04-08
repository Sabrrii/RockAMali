#!/bin/bash

ns=123

list=
for g in copy program lambda closure function function_lambda function_macro program_template program_T4 program_T4xyzw program_T4ls_fma
do
  ./process_sequential -s 8192 -o sample_sequential.nc -r result_sequential.nc --generator-factory count -n $ns --use-GPU --GPU-factory $g
  ncks -A profiling_gpu.nc profiling_process.nc
  fo=profiling_GPU_$g.nc
  mv profiling_process.nc $fo
  list=$list" "$fo
done

#ensemble cat
ncecat -O $list -o profiling_GPU.nc
