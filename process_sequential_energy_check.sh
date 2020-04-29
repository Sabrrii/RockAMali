#!/bin/bash

#usage: ./process_sequential_energy_check.sh
#note: should be run after make process_sequential_run

ncrename -d dim1,dimS pac_signal_parameters.nc
ncdiff -O result_sequential.nc pac_signal_parameters.nc -o process_sequential_energy_check_diff.nc
ncdump process_sequential_energy_check_diff.nc
grep -e 'signal:A=' -e 'signal:B=' parameters.cdl --color

