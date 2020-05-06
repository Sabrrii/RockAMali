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

# use

cf. [`help.output`](udp_receive_help.output)

e.g.

~~~ { .bash }
make udp_receive_run
~~~

## external tools

- [`NCO`]() (NetCDF)
- [`NCview`]() (NetCDF)

