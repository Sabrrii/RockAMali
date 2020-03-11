//CoolImage
#include "CImg.h"
//C++ base
#include <iostream>
#include <string>
#include <vector>

//C base
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

// UDP point to point test

#define VERSION "v0.1.2m"

using namespace cimg_library;

//! CLI option parse and ...
int main(int argc, char **argv)
{
  ///command arguments, i.e. CLI option
  cimg_usage(std::string("send UDP frame.\n" \
  " It uses different GNU libraries (see --info option)\n\n" \
  " usage: ./send -h\n" \
  "        ./send -s 1024 -n 123\n" \
  "\n version: "+std::string(VERSION) + 
#ifdef DO_NETCDF
  "\n          CImg_NetCDF."+std::string(CIMG_NETCDF_VERSION) + 
#endif //NetCDF
  "\n compilation date:" \
  ).c_str());//cimg_usage

  unsigned int width=cimg_option("-s",8192, "size of UDP buffer [byte]");
  const int unsigned long max_iter=cimg_option("-n",256,  "number of frames");
  const bool endian_swap=!cimg_option("--no-endian-swap",false,"do not swap endianess, by default it is done if needed (arch. dep.)");
  const bool verbose=cimg_option("--verbose",false,"Produce verbose output");
  const bool debug=cimg_option("--debug",false,"debug output");
  const unsigned short port=cimg_option("-p",20485,"port where the packets are send on the receiving device");
  const std::string ip=cimg_option("-i", "10.10.17.202", "ip address of the receiver");
  const int twait=cimg_option("-w", 123, "waiting time between udp frames [us]");
  const bool do_warmup_W=cimg_option("-W",false,NULL);//-W hidden option
  bool do_warmup=cimg_option("--do-warmup",do_warmup_W,"do data warmup, e.g. allocation and fill (or -W option)");do_warmup=do_warmup_W|do_warmup;//same --do-warmup or -W option
  const bool do_ramp_R=cimg_option("-R",false,NULL);//-R hidden option
  bool do_ramp=cimg_option("--do-ramp",do_ramp_R,"do rate ramp on first 256 frames, e.g. ethernet rate raise to requested (or -R option)");do_ramp=do_ramp_R|do_ramp;//same --do-ramp or -R option

  ///standard options
  #if cimg_display!=0
  const bool show_X=cimg_option("-X",false,NULL);//-X hidden option
  bool show=cimg_option("--show",show_X,"show GUI (or -X option)");show=show_X|show;//same --show or -X option
  #endif
  const bool show_h   =cimg_option("-h",    false,NULL);//-h hidden option
        bool show_help=cimg_option("--help",show_h,"help (or -h option)");show_help=show_h|show_help; //same --help or -h option
  bool show_info=cimg_option("-I",false,NULL);//-I hidden option
  if( cimg_option("--info",show_info,"show compilation options (or -I option)") ) {show_info=true;cimg_library::cimg::info();}//same --info or -I option
  bool show_version=cimg_option("-v",false,NULL);//-v hidden option
  if( cimg_option("--version",show_version,"show version (or -v option)") )
  {
    show_version=true;
    std::cout<<VERSION<<std::endl;
#ifdef DO_NETCDF
    std::cout<<"  CImg_NetCDF."<<CIMG_NETCDF_VERSION<<std::endl;
    std::cout<<"  CParameterNetCDF."<<CDL_PARAMETER_VERSION<<std::endl;
    std::cout<<"  NcTypeInfo."<<NETCDF_TYPE_INFO_VERSION;
#endif //NetCDF
    std::cout<<std::endl;return 0;
  }//same --version or -v option

  if(show_help) {/*print_help(std::cerr);*/return 0;}
  //}CLI option

  //! content buffer (as char)
  CImg<unsigned char> buffer(width);
  //! buffer as index (shared with buffer), i.e. cast to uint32, but still in net endian !
  CImg<unsigned int>  bindex(buffer.width()/4,buffer.height(),buffer.depth(),buffer.spectrum());
  const unsigned int* bindex_data=bindex._data;//keep memory of allocation place, before get shared data (for freeing)
  bindex._data=(unsigned int*)buffer.data();//share

  //UDP related
  int clientSocket, portNum;
  struct sockaddr_in receiverAddr;
  socklen_t addr_size;
  //Create UDP socket
  clientSocket = socket(PF_INET, SOCK_DGRAM, 0);
  //Configure settings in address struct
  receiverAddr.sin_family = AF_INET;
  receiverAddr.sin_port = htons(port);
  receiverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
  memset(receiverAddr.sin_zero, '\0', sizeof receiverAddr.sin_zero);  
  //Initialize size variable to be used later on
  addr_size = sizeof receiverAddr;

  if(verbose) if(!endian_swap) printf("information: NO swap endianess\n");
  //randomise content of buffer
  if(verbose) printf("rnd\r");
  buffer.rand(0,255);
  if(do_warmup) {printf("information: do memory warmup.\n");buffer.rand(0,255);bindex.max();}
  if(do_ramp)   {printf("information: do rate ramp on first 256 frames.\n");}

//  while(1)
  for(int i=0;i<max_iter;++i)
  {
    printf("i=%d\r",i);if(i==0) fflush(stdout);
    //update loop index in UDP buffer
    bindex(0)=(!endian_swap)?i:ntohl(i);
    //send buffer to receiver
    sendto(clientSocket,buffer,width,0,(struct sockaddr *)&receiverAddr,addr_size);
    //control data rate
    if(do_ramp) {if(i<256) {const int tw=twait*(256-i);if(verbose) printf("wait=%dus\n",tw); usleep(tw);} else usleep(twait);}
    else usleep(twait);
  }//for loop
  printf("\n");
  //put back memory pointer (for freeing)
  bindex._data=(unsigned int*)bindex_data;
  return 0;
}//main

