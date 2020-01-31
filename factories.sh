#!/bin/bash

for f in process  process_sequential  receive  send
do
  /bin/echo -n $f.
  ./$f --version | grep 'v.\..\..'
  ./$f --help    | grep 'factory' | sed 's/information://'
done 2>/dev/null

