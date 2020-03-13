#!/bin/bash

ip=10.10.16.1
ip=$1

fi=/tmp/4GB.rnd
fr=/dev/shm/4GB.rnd
fo=nc_send.txt

#copy file to RAM
rm /dev/shm/*GB.rnd; rsync -var $fi $fr --progress

#send it (with elapsed time and rate)
echo 'nc send '$fr > $fo
t0=`date +%s`; echo ${t0}
cat $fr | nc -v -4 $ip 12345 --send-only
t1=`date +%s`; echo ${t1}; dt=`expr ${t1} - ${t0}`
echo $dt's  '`expr 4096 / ${dt}`'MB/s' | tee -a $fo

