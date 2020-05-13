#!/bin/bash

#temporary stuff for CI, i.e. within Linux docker container

pwd
ls ..
ls ../*

di=ci-temporary

#dev. libs
##CImg
if [ ! -e ../CImg ]
then
  mv $di/CImg ../
else
  rm -fr $di/CImg
fi

##NetCDF
for d in NetCDF NetCDF.Tool CImg.Tool
do
  if [ ! -e ../$d ]
  then
    mv $di/$d ../
  else
    rm -fr $di/$d
  fi
done

#check NetCDF bin
ncgen parameters.cdl -o parameters.nc

