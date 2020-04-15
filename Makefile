#run
## ushort = 2uchar: 4096*2 = 8192BoF
## uint   = 4uchar: 2048*2 = 8192BoF
##RockAMali <- gansacq2
FRAME_SIZE=8192
WAIT4RATE=256
DELTAWAIT=128
RECO=--crc false -B 2 -w 1
RECO=-w 1
RAMP=--do-ramp -r 128
#RAMP=
IPERF=--bandwidth 1G --udp
DST_IP=10.10.15.1
ETH=enp1s0
CPU_AFFINITY="0"
###ganp484 <- gansacq2 (10GEth copper)
#DST_IP=10.10.16.1
#ETH=eth1
##ml507 -> ganp157
#FRAME_SIZE=256
#DST_IP=10.10.17.202
#ETH=p1p2
PORT=20485
NS=23
NB=12
NP=2
GEN_FCT=peak_noise
PROC=filter
USE_GPU=--use-GPU --GPU-factory program_image1d
USE_GPU=
DO_CHECK=--do-check --do-fill
#DO_CHECK=
DO_PROFILING=-DDO_PROFILING
#DO_PROFILING=

DATA=./
DATA=/media/temp/
#DATA=/tmp/
DIN=samples/
DOUT=results/
FIN=sample.cimg
#FIN=sample.nc
FOUT=$(FIN)

#compiler options
LIB_XWINDOWS=-I/usr/X11R6/include -L/usr/X11R6/lib -lX11
LIB_CIMG=-I../CImg -Wall -W -pedantic -Dcimg_use_vt100 -lpthread -lm -fopenmp
LIB_BOOST_ASIO=-lboost_system
LIB_BOOST_COMPUTE=-lMali -L/usr/lib/aarch64-linux-gnu/ -DBOOST_COMPUTE_MAX_CL_VERSION=102
#NetCDF library (depending on target architecture)
ifeq ($(shell uname -p),x86_64)
##AMD64 (gan*)
	LIB_NETCDF= -I../NetCDF/include/ -lnetcdf_c++ -L../NetCDF/lib/ -lnetcdf -I../NetCDF.Tool/ -I../CImg.Tool/
else
##ARM64 (RockPro64)
	LIB_NETCDF= -I/usr/include/ -lnetcdf_c++ -L/usr/lib/aarch64-linux-gnu/ -lnetcdf -I../NetCDF.Tool/ -I../CImg.Tool/
endif #NetCDF

##do compile
DO_NETCDF=-DDO_NETCDF $(LIB_NETCDF)
#DO_NETCDF=
#DO_GPU (depending on target architecture)
ifeq ($(shell uname -p),x86_64)
##AMD64 (gan*)
	DO_GPU=
	DO_GPU_PROFILING=
        DO_CPU_AFFINITY=
else
##ARM64 (RockPro64)
	DO_GPU=-DDO_GPU $(LIB_BOOST_COMPUTE)
	DO_GPU_PROFILING=-DDO_GPU_PROFILING
        DO_CPU_AFFINITY=-DDO_CPU_AFFINITY
endif #DO_GPU

#source package
SRC_DATA_BUFFER=thread_lock.hpp CDataAccess.hpp CDataBuffer.hpp
SRC_DATA_GENERATOR=CDataGenerator.hpp CDataGenerator_factory.hpp
SRC_DATA_PROCESS=CDataProcessor.hpp CDataProcessor_vMvPv.hpp CDataProcessorGPU.hpp CL_image_data_type.h CDataProcessorGPUopencl.hpp CDataProcessorGPU_vMcPc.hpp CDataProcessorGPUfactory.hpp CProfiling.hpp
HELP_OUTPUT=process_sequential_help.output process_help.output send_help.output receive_help.output store_help.output
SRC_NETCDF=../NetCDF.Tool/NetCDFinfo.h ../NetCDF.Tool/struct_parameter_NetCDF.h ../CImg.Tool/CImg_NetCDF.h

##RockAMali
#all: process_sequential process send receive version factory doc
#all: process_sequential process version factory doc
#all: send receive version factory doc
all: process_sequential version factory process_sequential_run
#all: send receive version factory

##lib. code test
#all: time_copy
#all: udp_receive udp_send
time_copy: time_copy.cpp Makefile
	g++ -O0 -o time_copy time_copy.cpp $(DO_GPU) && ./time_copy

std_high_res_clock: std_high_res_clock.cpp Makefile
	g++ -std=c++11 std_high_res_clock.cpp -o std_high_res_clock && ./std_high_res_clock

udp_receive: udp_receive.cpp Makefile
#	g++ -Wall udp_receive.cpp -o udp_receive && ./udp_receive --help && ./udp_receive -i 12 -D -s --verbose
	g++ -O0 -o udp_receive  udp_receive.cpp $(LIB_CIMG) $(DO_NETCDF) $(DO_CPU_AFFINITY) -Dcimg_display=0 $(DO_GPU) $(DO_GPU_PROFILING) && ./udp_receive --help -I && ./udp_receive -v > VERSION && ./udp_receive -n 12 -w 1 --debug --simulation --verbose
#	g++ -O0 udp_receive.X  udp_receive.cpp $(LIB_CIMG) $(DO_NETCDF) $(LIB_XWINDOWS)  $(DO_GPU) $(DO_GPU_PROFILING) && ./udp_receive.X -h -I && ./udp_receive.X -v > VERSION
	@echo "sync; make && make udp_receive_run 2>&1 | tee udp_receive.txt"
udp_receive_run:
	/sbin/ifconfig $(ETH) | grep RX | grep dropped #; ethtool -S $(ETH) | grep InDropped --color
	env GOMP_CPU_AFFINITY=$(CPU_AFFINITY) ./udp_receive -s `echo $(FRAME_SIZE)*4 | bc` -i $(DST_IP) -p $(PORT) -n $(NS) $(DO_CHECK) --do-warmup $(RECO) 2>&1 | tee udp_receive.txt
	ncdump -h udp_receive.nc | tee udp_receive.cdl ;ncdump -h udp_receive_rate.nc | tee udp_receive_rate.cdl ; ncdump -h udp_receive_drop.nc | tee udp_receive_drop.cdl
	/sbin/ifconfig $(ETH) | grep RX | grep dropped #; ethtool -S $(ETH) | grep InDropped --color
	@echo "sync; make && make udp_receive_run 2>&1 | tee udp_receive.txt"
udp_send: udp_send.cpp Makefile
	g++ -O0 -o udp_send  udp_send.cpp $(LIB_CIMG) $(DO_NETCDF) -Dcimg_display=0 $(DO_GPU) $(DO_GPU_PROFILING) && ./udp_send --help -I && ./udp_send -v > VERSION && ./udp_send -n 12
udp_send_run:
	/sbin/ifconfig eth6 | grep TX | grep dropped
	./udp_send -s `echo $(FRAME_SIZE)*4 | bc` -i $(DST_IP) -p $(PORT) -n `echo $(NS)+1 | bc` -w $(WAIT4RATE) -dw $(DELTAWAIT) --do-warmup $(RAMP) $(DO_CHECK) --verbose
	ncdump -h udp_send.nc | tee udp_send.cdl
	/sbin/ifconfig eth6 | grep TX | grep dropped

gui: main.cpp
	g++ -O0 -o generate.X main.cpp -I../CImg -Wall -W -ansi -pedantic -Dcimg_use_vt100 -lpthread -lm -fopenmp -lboost_system $(LIB_XWINDOWS) && ./generate.X -h -I && ./generate.X -v > VERSION
	./generate.X -h 2> generateX_help.output

process: process.cpp SDataTypes.hpp $(SRC_DATA_BUFFER) $(SRC_DATA_GENERATOR) $(SRC_DATA_PROCESS) CDataStore.hpp $(SRC_NETCDF)
	g++ -O0 -o process   process.cpp $(LIB_CIMG) $(DO_NETCDF) -Dcimg_display=0 $(DO_GPU) $(DO_GPU_PROFILING) && ./process   -h -I && ./process   -v > VERSION
#	g++ -O0 -o process.X process.cpp $(LIB_CIMG) $(DO_NETCDF) $(LIB_XWINDOWS)  $(DO_GPU) $(DO_GPU_PROFILING) && ./process.X -h -I && ./process.X -v > VERSION
	./process -h 2> process_help.output

#SEQ_GPU=
#SEQ_GPU=-DDO_GPU_SEQ_QUEUE
SEQ_GPU=-DDO_GPU_NO_QUEUE
process_sequential: process_sequential.cpp SDataTypes.hpp $(SRC_DATA_BUFFER) $(SRC_DATA_GENERATOR) $(SRC_DATA_PROCESS) CDataStore.hpp $(SRC_NETCDF)
	g++ -std=c++11 $(SEQ_GPU) -O0 -o process_sequential   process_sequential.cpp $(LIB_CIMG) $(DO_NETCDF) -Dcimg_display=0 $(DO_GPU) $(DO_GPU_PROFILING) $(DO_PROFILING) && ./process_sequential   -h -I && ./process_sequential   -v > VERSION
#	g++ -std=c++11 $(SEQ_GPU) -O0 -o process_sequential.X  process_sequential.cpp $(LIB_CIMG) $(DO_NETCDF) $(LIB_XWINDOWS)  $(DO_GPU) $(DO_GPU_PROFILING) $(DO_PROFILING) && ./process_sequential.X -h -I && ./process_sequential.X -v > VERSION
	./process_sequential -h 2> process_sequential_help.output

send: send.cpp SDataTypes.hpp $(SRC_DATA_BUFFER) $(SRC_DATA_GENERATOR) CDataSend.hpp $(SRC_NETCDF)
	g++ -O0 -o send   send.cpp  $(LIB_CIMG) $(LIB_BOOST_ASIO) $(DO_NETCDF) -Dcimg_display=0 && ./send -h -I && ./send -v > VERSION
	./send -h 2> send_help.output

receive: receive.cpp SDataTypes.hpp $(SRC_DATA_BUFFER) CDataReceive.hpp $(SRC_DATA_PROCESS) CDataStore.hpp
	g++ -O0 -o receive receive.cpp  $(LIB_CIMG) $(LIB_BOOST_ASIO) $(DO_NETCDF) -Dcimg_display=0 $(DO_GPU) $(DO_GPU_PROFILING) && ./receive -h -I && ./receive -v > VERSION
	./receive -h 2> receive_help.output

receive_sequential: receive_sequential.cpp SDataTypes.hpp $(SRC_DATA_BUFFER) CDataReceive.hpp $(SRC_DATA_PROCESS) CDataStore.hpp
	g++ -O0 -o receive_sequential receive_sequential.cpp  $(LIB_CIMG) $(LIB_BOOST_ASIO) $(DO_NETCDF) -Dcimg_display=0 $(DO_GPU) $(DO_GPU_PROFILING) && ./receive_sequential -h -I && ./receive -v > VERSION
	./receive -h 2> receive_help.output

doc: doxygen.sh Doxyfile.template doxygen.cpp VERSION VERSIONS $(HELP_OUTPUT) process.cpp process_sequential.cpp send.cpp receive.cpp  $(SRC_DATA_BUFFER) CDataReceive.hpp $(SRC_DATA_GENERATOR) $(SRC_DATA_PROCESS) CDataStore.hpp
	./doxygen.sh

version: versions.sh process.cpp process_sequential.cpp send.cpp receive.cpp
	./versions.sh > VERSIONS; cat VERSIONS; echo

factory: factories.sh CDataGenerator.hpp $(SRC_DATA_BUFFER)
	./factories.sh 2>&1 > FACTORIES; cat FACTORIES; echo

#NP=4
NT=`echo $(NP)+2   | bc`
#NB=`echo $(NP)*4096| bc`
#NS=`echo $(NP)*8192| bc`

#NB=`echo $(NP)*4| bc`
#NS=`echo $(NP)*8| bc`
process_run:
	ncgen parameters.cdl -o parameters.nc && rm -f sample.nc; ./process -c $(NT) -s $(FRAME_SIZE) -o sample.nc --generator-factory $(GEN_FCT) --CPU-factory $(PROC) -b $(NB) -n $(NS) $(USE_GPU) $(DO_CHECK) 2>&1 | grep -e info -e test -e failed -e double -e fault -e $(GEN_FCT) -e $(PROC) --color && ncdump -h sample.nc
#	./process -c $(NT) -s $(FRAME_SIZE) -o sample.cimg --generator-factory $(GEN_FCT) --CPU-factory $(PROC) $(USE_GPU) -b $(NB) -n $(NS) $(DO_CHECK) # 2>&1 | grep -e info -e test -e failed -e double -e fault -e $(GEN_FCT) -e $(PROC) --color

process_sequential_run:
	ncgen parameters.cdl -o parameters.nc && rm -f sample_sequential.nc result_sequential.nc; ./process_sequential   -s $(FRAME_SIZE) -o sample_sequential.nc -r result_sequential.nc --generator-factory $(GEN_FCT) --CPU-factory $(PROC) -n $(NS) $(USE_GPU) $(DO_CHECK) && ncdump -h sample_sequential.nc && ncdump -h result_sequential.nc && ncdump -h profiling_gpu.nc && ncdump -h profiling_process.nc #&& ncview sample_sequential.nc
#	ncgen parameters.cdl -o parameters.nc && rm sample_sequential.nc; ./process_sequential.X -s $(FRAME_SIZE) -o sample_sequential.nc --generator-factory $(GEN_FCT) --CPU-factory $(PROC) -n $(NS) $(USE_GPU) $(DO_CHECK) --show && ncdump -h sample_sequential.nc

process_sequential_check: result_sequential.nc  sample_sequential.nc
	ncrename -v signal,process result_sequential.nc
	ncview sample_sequential.nc& ncview result_sequential.nc&
	cp -p  sample_sequential.nc  all_sequential.nc && ncks -A result_sequential.nc -o all_sequential.nc && ncdump -h all_sequential.nc
	ncap2 -s "ncoprocess=signal*2.1+123.45; diff=process-ncoprocess" all_sequential.nc -o diff_sequential.nc --overwrite && ncdump -h diff_sequential.nc
	ncview diff_sequential.nc &

#data check for all factory kernels
process_sequential_vMcPc_check:
	./process_sequential_vMcPc_check.sh | tee process_sequential_vMcPc_check.txt | grep -e ' ' -e fail --color

send_run:
	./send    -c 2 -s $(FRAME_SIZE) -p $(PORT) -i $(DST_IP) -b  8 -n `echo $(NS)+1 | bc` -w 123456

#NT=`echo $(NP)+3   | bc`
#NB=`echo $(NP)*4096| bc`
receive_run: clear
#	./receive -c 2 -s $(FRAME_SIZE) -b 128 -n 12345 -o $(DATA)$(DIN)$(FIN) -r $(DATA)$(DOUT)$(FOUT) -C -E -W
#	./receive -c 3 -s $(FRAME_SIZE) -b 16 -n 123 -o $(DATA)$(DIN)$(FIN) -r $(DATA)$(DOUT)$(FOUT) $(USE_GPU) $(DO_CHECK) -E -W
#	./receive -c 4 -s $(FRAME_SIZE) -b 16 -n 123 -o $(DATA)$(DIN)$(FIN) -r $(DATA)$(DOUT)$(FOUT) $(USE_GPU) $(DO_CHECK) -E -W
	./receive -c $(NT) -s $(FRAME_SIZE) -p $(PORT) -b $(NB) -n $(NS) -o $(DATA)$(DIN)$(FIN) -r $(DATA)$(DOUT)$(FOUT) $(USE_GPU) $(DO_CHECK) -W

receive_sequential_run: clear
	./receive_sequential -c $(NT) -s $(FRAME_SIZE) -p $(PORT) -b $(NB) -n $(NS) -o $(DATA)$(DIN)$(FIN) -r $(DATA)$(DOUT)$(FOUT) $(USE_GPU) $(DO_CHECK) -W

##ganp157
ganp_eth:
	/sbin/ifconfig p1p2
	ping 10.10.17.202 -c 1
# #interface: (su)
#iftop -B -i p1p2
#tshark   -i p1p2 -c 12 -x
#rm -f /tmp/UDP_ml507; tshark -i p1p2 -c 1  -x -w /tmp/UDP_ml507 ; wc -c /tmp/UDP_ml507

##rockpro64 (sudo apt-get install tshark #yes#; sudo usermod -a -G wireshark rock64)
rockpro64_eth:
	sudo /sbin/ifconfig enp1s0
	sudo /sbin/ifconfig enp1s0 10.10.15.1/24
	sudo /sbin/ifconfig enp1s0 down; sudo ethtool -s enp1s0 speed 10000 duplex full autoneg off; sudo /sbin/ifconfig enp1s0 up
	ping 10.10.15.2 -c 1
#rock64 (sudo apt-get install tshark #yes#; sudo usermod -a -G wireshark rock64)
#sudo iftop -B -i p1p2

#UPD grab{
FILTER=dst port $(PORT)
ifeq ($(shell uname -p),x86_64)
##AMD64 (gan*)
TMP=/tmp/
TMP=/space/users/coudert/temp/
TMP=/dev/shm/
udp_grab:
	tshark -i p1p2 -f "$(FILTER)" -c $(NS) -x | grep  -e port -e 0000 -e 0010 -e 0020 -e 0420
	@echo "rm -f $(TMP)/UDP_ml507; tshark -i p1p2 -f \"$(FILTER)\" -c 1  -x -w $(TMP)/UDP_ml507; chmod a+r $(TMP)/UDP_ml507; wc -c $(TMP)/UDP_ml507"
	@echo "rm -f $(TMP)/UDP_ml507_grab; tshark -i p1p2 -f \"$(FILTER)\" -c 1234567 -w $(TMP)/UDP_ml507_grab; chmod a+r $(TMP)/UDP_ml507_grab; wc -c $(TMP)/UDP_ml507_grab"
else
##ARM64 (RockPro64)
TMP=/media/temp/
udp_grab:
	tshark -i enp1s0 -f "$(FILTER)" -c $(NS) #-x
	@echo "rm -f $(TMP)/UDP_ganl; tshark -i enp1s0 -f \"$(FILTER)\" -c 1  -x -w $(TMP)/UDP_ganl ; wc -c $(TMP)/UDP_ganl"
endif #UPD grab
#}UPD grab


##gansacq2 (su)
gansacq2_eth:
	/sbin/ifconfig eth7
	/sbin/ifconfig eth7 10.10.16.2/24
	/sbin/ifconfig eth7 down; ethtool -s eth7 speed 10000 duplex full autoneg off; /sbin/ifconfig eth7 up
	ping 10.10.16.1 -c 1
	@echo iftop -B -i eth7

##ganp484 (su)
ganp484_eth:
	/sbin/ifconfig eth1
	/sbin/ifconfig eth1 10.10.16.1/24
	/sbin/ifconfig eth1 down; ethtool -s eth1 speed 10000 duplex full autoneg off; /sbin/ifconfig eth1 up
	ping 10.10.16.2 -c 1
	@echo iftop -B -i eth1

##ganp484/RockAMali <- gansacq2
#gansacq2
nc_send_prepare: /tmp/4GB.rnd
	./nc_send_file.sh
nc_send_run:
	./nc_send.sh $(DST_IP)
#ganp484 or RockAMali
nc_receive_run:
	nc -v -l -p 12345 > /dev/null

##ganp484/RockAMali <- gansacq2
#gansacq2
iperf_send_run:
	iperf3 -c $(DST_IP) -p $(PORT) -4 -w `echo $(FRAME_SIZE)*4 | bc` -k $(NS) $(IPERF) | tee network_iperfc.txt
#ganp484 or RockAMali
iperf_receive_run:
	iperf3 -s -p $(PORT) -1 | tee network_iperfs.txt


clear:
	rm -fr $(DATA)/samples/ $(DATA)/results/
	mkdir  $(DATA)/samples/ $(DATA)/results/
	rm -f sample_??????.cimg result_??????.cimg
	rm -f sample.nc sample_sequential.nc parameters.nc
	sync

clean:
	make clear
	rm -f send.X    send
	rm -f receive.X receive
	rm -f process.X process
	rm -f process_sequential.X process_sequential

display:
	convert -append $(DATA)/samples/sample*.png $(DATA)/samples.png && display samples.png &
	convert -append $(DATA)/results/sample*.png $(DATA)/results.png && display results.png

