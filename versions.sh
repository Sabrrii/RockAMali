#!/bin/bash

for f in process  process_sequential  receive  send  store
do
  /bin/echo -n $f.
  ./$f --version | grep 'v.\..\..'
done

