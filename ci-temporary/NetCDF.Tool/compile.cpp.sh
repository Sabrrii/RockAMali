#!/bin/bash
f=$1
fb=`basename $f .cpp`
g++ -m64 -g -O2 $f -I../NetCDF/include/ -lnetcdf_c++ -L../NetCDF/lib/ -lnetcdf -o $fb

exit
g++ -m32 -g -O2 simple_xy_wr.cpp -I../NetCDF/include/ -lnetcdf_c++ -L../NetCDF/lib/ -lnetcdf -o simple_xy_wr
./simple_xy_wr
../NetCDF/bin/ncdump simple_xy.nc

exit
#or
g++ -m32 -c -g -O2 -I../NetCDF/include/ -I. simple_xy_wr.cpp
g++ -m32 -o simple_xy_wr -g -O2  simple_xy_wr.o -lnetcdf_c++ -L../NetCDF/lib/ -lnetcdf

