#usage: make write && make write_run

#make file
LIB_XWINDOWS=-I/usr/X11R6/include -L/usr/X11R6/lib -lX11
LIB_CIMG=-I../CImg -Wall -W -ansi -pedantic -Dcimg_use_vt100 -lpthread -lm -fopenmp
#NetCDF library (depending on target architecture)
ifeq ($(shell uname -p),x86_64)
##AMD64 (gan*)
	LIB_NETCDF= -I../NetCDF/include/ -lnetcdf_c++ -L../NetCDF/lib/ -lnetcdf
else
##ARM64 (RockPro64)
	LIB_NETCDF= -I/usr/include/ -lnetcdf_c++ -L/usr/lib/aarch64-linux-gnu/ -lnetcdf
endif #NetCDF
CPP= g++ -O0 -Wall -W 

all: write read version

version: writeCImgNetCDF_test.cpp readCImgNetCDF_test.cpp CImg_NetCDF.h ../CImg/CImg.h
	./writeCImgNetCDF_test --version > VERSIONS
	./readCImgNetCDF_test --version >> VERSIONS
	grep cimg_version ../CImg/CImg.h --color | head -n 2 | tail -n 1 | sed 's/#define //' >> VERSIONS
	cat VERSIONS | sort -u -r > VERSIONS.uniq && mv VERSIONS.uniq VERSIONS
	cat VERSIONS

write: writeCImgNetCDF_test.cpp CImg_NetCDF.h  useCImg.h
	$(CPP) writeCImgNetCDF_test.cpp $(LIB_CIMG) $(LIB_NETCDF) -o writeCImgNetCDF_test && ./writeCImgNetCDF_test --help --version

read: readCImgNetCDF_test.cpp CImg_NetCDF.h  useCImg.h
	$(CPP) readCImgNetCDF_test.cpp $(LIB_CIMG) $(LIB_NETCDF) -o readCImgNetCDF_test && ./readCImgNetCDF_test --help --version

write_run:
	./writeCImgNetCDF_test -x 4096 -y 1 -z 1 -v 1 -t 12
	ncdump -h CImgNetCDF_CImgTest.nc | tee CImgNetCDF_CImgTest.cdl
	ncdump -h CImgNetCDF_CImgListTest.nc | tee CImgNetCDF_CImgListTest.cdl
	ncdump -h CImgNetCDF_CImgListnCImgTest.4D+1.nc | tee CImgNetCDF_CImgListnCImgTest.4D+1.cdl

read_run:
	ncdump -h CImgNetCDF_cimgListTest.4d-1.nc && ./readCImgNetCDF_test

run: write_run read_run

clear:
	rm -f CImgNetCDF_CImgTest.nc CImgNetCDF_CImgListTest.nc CImgNetCDF_CImgListnCImgTest.4D+1.nc
	sync

clean: clear
	rm -f writeCImgNetCDF_test readCImgNetCDF_test

