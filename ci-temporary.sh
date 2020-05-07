#!/bin/bash

#temporary stuff for CI, i.e. within Linux docker container

pwd
ls ..
ls ../*

#dev. libs
##CImg
if [ ! -e ../CImg ]
then
  mv CImg ..
else
  rm -fr CImg
fi

##NetCDF
#TODO
ncgen parameters.cdl -o parameters.nc

