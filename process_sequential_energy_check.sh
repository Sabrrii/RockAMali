#!/bin/bash

#usage: ./process_sequential_energy_check.sh

gen="signal_pac_rnd"
proc_list="pac filter"
ns=23

statistics='maximum minimum mean'

#short names for statistics
#stats='maxi mini mean'
stats=
for o in $statistics
do
  stats=$stats' '${o:0:4}
done

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
  ##show
  ncdump $fo
  grep -e 'signal:A=' -e 'signal:B=' parameters.cdl --color

  #statistics
  list=
  i=1
  for o in $statistics
  do
    fo=$fb'_'$o.nc
    list=$list' '$fo
    ncwa -y $o -O $fb'_diff.nc' -o $fo
    var=E_`echo $stats | cut -d' ' -f$i`
    ncrename -v E,$var $fo
    ncatted -a long_name,$var,m,c,"$o energy" $fo
    ((++i))
    ##show
    ncdump $fo
  done
  ##gather
  fo=process_sequential_energy_check_stats_$p.nc
  rm $fo
  for f in $list
  do
    ncks -A $f -o $fo
  done
  ##clear history
  ncatted --history -a history,global,d,, -a history_of_appended_files,global,d,, $fo
  ##show
  ncdump $fo
  ##next loop
  stat_list=$stat_list' '$fo
done

for f in $stat_list
do
  ncdump $f
done

#check close to zero
##prepare
fo=parameters_stats.nc
###extract E
E=`ncdump -v signal parameters.nc | grep ':A ' | cut -d':' -f2 | sed 's/A/E/' | sed 's/ ;/f ;/'`
ncap2 -O -s "$E" parameters.nc -o tmp.nc
ncks -O -v E tmp.nc -o parameters_E.nc
rm tmp.nc
###stat E
rm parameters_stats.nc
for o in $stats
do
  var=E_$o
  ncrename -O -v E,$var parameters_E.nc -o tmp.nc
  ncks -A tmp.nc -o $fo
done
rm tmp.nc parameters_E.nc
##clear history
ncatted --history -a history,global,d,, -a history_of_appended_files,global,d,, $fo
##show
ncdump $fo

##%
for f in $stat_list
do
  fo=`basename $f .nc`_percentage.nc
  #vars='E_maxi*=100;E_mini*=100;E_mean*=100;'
  vars=
  for o in $stats
  do
    vars=$vars'E_'$o'*=100; '
  done
  ncap2 -O -s "$vars" $f -o tmp.nc
  ncbo -O --op_typ='/' tmp.nc parameters_stats.nc -o $fo
  ##percentage
  for o in $stats
  do
    var=E_$o
    varp=E_$o'_%'
    ncrename -v $var,$varp $fo
    ncatted -a units,$varp,m,c,"%" $fo
    ncatted -a long_name,$varp,a,c," percentage" $fo
  done
  ##clear history
  ncatted --history -a history,global,d,, -a history_of_appended_files,global,d,, $fo
  ##show
  ncdump $fo
done
#clean
rm tmp.nc parameters_stats.nc

#o. do a loop on processor (CPU factory, GPU ...)



