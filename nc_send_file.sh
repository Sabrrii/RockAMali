#!/bin/bash

#random files
if [ 0 ]  ; then
dd bs=1 count=1024 if=/dev/random of=/tmp/1kB.rnd
for((i=0;i<1024;++i)); do dd bs=1 count=1024 if=/dev/random of=/tmp/1kB_$i.rnd; done; cat /tmp/1kB_*.rnd > /tmp/1MB.rnd; rm -f /tmp/1kB_*.rnd
rm -f /tmp/1GB.rnd; for((i=0;i<1024;++i)); do cat /tmp/1MB.rnd >> /tmp/1GB.rnd; done
fi #/dev/random

if [ 1 ]  ; then
dd bs=1  count=1024 if=/dev/urandom of=/tmp/1kB.rnd
dd bs=1k count=1024 if=/dev/urandom of=/tmp/1MB.rnd
dd bs=1M count=1024 if=/dev/urandom of=/tmp/1GB.rnd
fi #/dev/urandom

ls -lah /tmp/1?B.r??

#big files
rm -f /tmp/4GB.rnd;  cat /tmp/1GB.rnd /tmp/1GB.rnd /tmp/1GB.rnd /tmp/1GB.rnd > /tmp/4GB.rnd
rm -f /tmp/8GB.rnd;  cat /tmp/4GB.rnd /tmp/4GB.rnd > /tmp/8GB.rnd
rm -f /tmp/16GB.rnd; cat /tmp/8GB.rnd /tmp/8GB.rnd > /tmp/16GB.rnd
ls -lah /tmp/?GB.rnd

