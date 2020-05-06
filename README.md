status (branch: dev_udp_ml507):

[![pipeline status](https://gitlab.in2p3.fr/RockPro64/RockAMali/badges/dev_udp_ml507/pipeline.svg)](https://gitlab.in2p3.fr/RockPro64/RockAMali/commits/dev_udp_ml507)


[![build status](https://gitlab.in2p3.fr/RockPro64/RockAMali/badges/dev_udp_ml507/build.svg)](https://gitlab.in2p3.fr/RockPro64/RockAMali/commits/dev_udp_ml507)


# description

WiP

# compile

## external library

- [`CImg`](http://www.cimg.eu/)
     - XWindows via `X11` development library for window output

~~~ { .bash }
sudo apt-get install cimg-dev cimg-doc cimg-examples libxrandr-dev
~~~

- [`NetCDF`]()

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

- [`NCO`]() (NetCDF)
- [`NCview`]() (NetCDF)

