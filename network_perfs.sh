#!/bin/bash

 bof=8192
bof4=2048

WAIT4RATE=12

arch=`uname --processor`
#if [ $arch == 'aarch64' ]
#then
#  do_send=false
#else
#  do_send=true
#fi
host=`cat /etc/hostname`
if [ $host == 'gansacq2' ]
then
  do_send=true
else
  do_send=false
fi

for bof4 in 0128 0256 0512 1024 2048 4096 8192 2048
do
  bof=`echo $bof4*4 | bc`
  echo $bof"BoF"
  sed -i "s/FRAME_SIZE=..../FRAME_SIZE=$bof4/" Makefile
  if $do_send
  then
    sync; make udp_send_run
    sleep 12
  else
    make udp_receive_run | grep -v '#' #grep for low band width
    fo=network_perfs.txt
    /bin/echo -e -n "\n\n${bof}BoF\n" >> $fo
    grep 'dt=' udp_receive.txt | tail -n 5 | head -n 2 >> $fo
    grep -e 'test' -e 'count' udp_receive.txt >> $fo
    sync; git commit -am "net. perfs: ${bof}BoF"
  fi
done

