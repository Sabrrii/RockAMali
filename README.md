status (branch: dev_udp_ml507):

[![pipeline status](https://gitlab.in2p3.fr/RockPro64/RockAMali/badges/dev_udp_ml507/pipeline.svg)](https://gitlab.in2p3.fr/RockPro64/RockAMali/commits/dev_udp_ml507)


[![build status](https://gitlab.in2p3.fr/RockPro64/RockAMali/badges/dev_udp_ml507/build.svg)](https://gitlab.in2p3.fr/RockPro64/RockAMali/commits/dev_udp_ml507)


# description

WiP

# compile

## C++

0. [`GNU compiler`](https://gcc.gnu.org/) `g++` and other tools, e.g. `make` or `C++::boost` for `BOOST_PP_STRINGIZE`

~~~ { .bash }
sudo apt-get install g++ build-essential libboost-dev
~~~

## external library

1. [`CImg`](http://www.cimg.eu/)
     - XWindows via `X11` development library for window output

array container (and debug display in window)

- minimal X install (then should use clone CImg in ../CImg/)

~~~ { .bash }
sudo apt-get install g++ libxrandr-dev
~~~

- full distribution install (this might install OpenCV that is very big and not used)

~~~ { .bash }
sudo apt-get install cimg-dev cimg-doc cimg-examples libxrandr-dev
~~~

2. [`NetCDF`](http://unidata.github.io/netcdf-cxx4/)

store result and log in compact NetCDF format

~~~ { .bash }
sudo apt-get install netcdf-bin libnetcdf-cxx-legacy-dev
~~~

3. optional `boost::compute`

**TODO**

to compile `OpenCL` for computing on **GPU**:

~~~ { .bash }
#sudo apt-get install boost::compute ...
#or
#git clone ...
~~~

## git clone

git for both RockAMali and linked libraries

~~~ { .bash }
cd code.RockAMali/
#CImg
git clone https://github.com/dtschump/CImg.git
#NetCDF
git clone git@gitlab.in2p3.fr:SebastienCOUDERT/NetCDF.git
##NetCDF tools
git clone git@gitlab.in2p3.fr:SebastienCOUDERT/NetCDF.Tool.git && cd NetCDF.Tool && git checkout RockAMali && cd ..
##CImg tools
git clone git@gitlab.in2p3.fr:SebastienCOUDERT/CImg.Tool.git   && cd CImg.Tool   && git checkout RockAMali && cd ..
git clone git@gitlab.in2p3.fr:RockPro64/RockAMali.git
#list all repository folders
ls -d CImg NetCDF NetCDF.Tool CImg.Tool RockAMali
#go in RockAMali one and onto targeted branch
cd RockAMali/
git checkout dev_udp_ml507 && make
~~~

## C++ compilation

cf. [`_info.txt`](_info.txt)

e.g.

~~~ { .bash }
make udp_receive
~~~

# doc

both user and developper documentation sources are in C++ code (e.g. [`doxygen.cpp`](doxygen.cpp))
, these are automatically compiled into HTML by CI pipeline, e.g. by `./doxgen.sh` or `make doc`.
It can be downloaded at the top of the present [GitLab page](https://gitlab.in2p3.fr/RockPro64/RockAMali/-/tree/dev_udp_ml507)
, press `Download` button (on the right) then `artifacts/doc`.
You'll get a `artifacts.zip` file containing the last doc. version
, it is in `doc/html/`:

- main   page: `doc/html/index.html`
- signal page: `doc/html/pageSchema.html`

The corresponding [GitLab wiki](https://gitlab.in2p3.fr/RockPro64/RockAMali/-/wikis/home)
 is also gathering these docs.

# use

cf. [`help.output`](udp_receive_help.output)

e.g.

~~~ { .bash }
make udp_receive_run
~~~

## UDP receive

relevant parameters of the `Makefile` are the following for `make udp_receive_run`:

~~~ { .bash }
##ml507 -> ganp157
FRAME_SIZE=2048
DST_IP=10.10.17.202
ETH=p1p2
PORT=20485
~~~

- `FRAME_SIZE`: frame size in `uint32` unit (i.e. 4 Bytes, annoted 4B), e.g. put `2048` for `8kBoF` (= `8192B` =`4*2048B`)
- `DST_IP`: destination IP address, e.g. `ganp` IP address
- `PORT`: UDP port, e.g. `20485`

misc.:

- `ETH`: ethernet device, e.g. `p1p2` (base on `/sbin/ifconfig`)

## external tools

- [`NCO`](https://www.unidata.ucar.edu/software/netcdf/docs/index.html) (NetCDF)
- [`NCview`](http://meteora.ucsd.edu/~pierce/ncview_home_page.html) (NetCDF)

