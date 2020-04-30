#!/bin/bash

#usage: ./process_sequential_energy_check.sh
#note: should be run after make process_sequential_run

ncrename -d dim1,dimS pac_signal_parameters.nc
ncdiff -O result_sequential.nc pac_signal_parameters.nc -o process_sequential_energy_check_diff.nc
ncdump process_sequential_energy_check_diff.nc
grep -e 'signal:A=' -e 'signal:B=' parameters.cdl --color

#check zero with nco
fb=process_sequential_energy_check
list=
for o in max min avg
do
  fo=$fb'_'$o.nc
  list=$list' '$fo
  ncwa -y $o -O $fb'_diff.nc' -o $fo
  var=E_$o
  ncrename -v E,$var $fo
  ncatted -a long_name,$var,m,c,"$o energy" $fo
  ncdump $fo
done

fo=process_sequential_energy_check_stats.nc
rm $fo
for f in $list
do
  ncks -A $f -o $fo
done
#clean history
ncatted --history -a history,global,d,, -a history_of_appended_files,global,d,, $fo
ncdump $fo

#o. do a loop on processor (CPU factory, GPU ...)

