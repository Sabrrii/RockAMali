#!/bin/bash

ip=10.10.16.1
ip=$1

sizeG=8
sizeM=`echo "$sizeG*1024" | bc`
fi=/tmp/$sizeG'GB.rnd'
fr=/dev/shm/$sizeG'GB.rnd'
fo=nc_send.txt

#copy file to RAM
rm /dev/shm/*GB.rnd; rsync -var $fi $fr --progress

#send it (with elapsed time and rate)
echo 'nc send '$fr > $fo
t0=`date +%s`; echo ${t0}
cat $fr | nc -v -4 $ip 12345 --send-only
t1=`date +%s`; echo ${t1}; dt=`expr ${t1} - ${t0}`
echo $dt's  '`expr ${sizeM} / ${dt}`'MB/s' | tee -a $fo
