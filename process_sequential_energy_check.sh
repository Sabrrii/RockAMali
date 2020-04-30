#!/bin/bash

#usage: ./process_sequential_energy_check.sh

gen=signal_pac_rnd
proc_list="pac filter"
ns=23

echo 'parse a few factory entities to compare to "'$gen'" simulation:'
stat_list=
for p in $proc_list
do
  echo 'processor '$p':'
  #run
  ./process_sequential -s 4096 -o sample_sequential.nc -r result_sequential.nc --generator-factory $gen --CPU-factory $p -n $ns #--use-GPU --GPU-factory

  #prepare
  ncrename -d dim1,dimS pac_signal_parameters.nc

  #difference
  fb=process_sequential_energy_check
  fo=$fb'_diff.nc'
  ncdiff -O result_sequential.nc pac_signal_parameters.nc -o $fo
  ncdump $fo
  grep -e 'signal:A=' -e 'signal:B=' parameters.cdl --color

  #statistics
  list=
  for o in maximum minimum mean
  do
    fo=$fb'_'$o.nc
    list=$list' '$fo
    ncwa -y $o -O $fb'_diff.nc' -o $fo
    var=E_$o #todo: cut to 4 char length
    ncrename -v E,$var $fo
    ncatted -a long_name,$var,m,c,"$o energy" $fo
    ncdump $fo
  done
  ###gather
  fo=process_sequential_energy_check_stats_$p.nc
  rm $fo
  for f in $list
  do
    ncks -A $f -o $fo
  done
  ###clean history
  ncatted --history -a history,global,d,, -a history_of_appended_files,global,d,, $fo
  ncdump $fo
  stat_list=$stat_list' '$fo
done

for f in $stat_list
do
  ncdump $f
done

#check close to zero

#o. do a loop on processor (CPU factory, GPU ...)

