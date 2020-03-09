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

//ArgP
#include <error.h>
#include <argp.h>

// UDP point to point test

//! \todo drop of exactly 2^32 should not be taken into drops
//! \todo add NetCDF for storing both frame index and increment in loop (unlimited dim.)
//! \todo tests: ml507, RockAMali, numexo2

#define VERSION "v0.1.2f"

using namespace cimg_library;

//! CLI option parse and ...
int main(int argc, char **argv)
{
  ///command arguments, i.e. CLI option
  cimg_usage(std::string("receive UDP frame.\n" \
  " It uses different GNU libraries (see --info option)\n\n" \
  " usage: ./receive -h\n" \
  "        ./receive -s 1024 -n 123 -o result.nc\n" \
  "\n version: "+std::string(VERSION) + 
#ifdef DO_NETCDF
  "\n          CImg_NetCDF."+std::string(CIMG_NETCDF_VERSION) + 
#endif //NetCDF
  "\n compilation date:" \
  ).c_str());//cimg_usage

  unsigned int width=cimg_option("-s",2048, "size of UDP buffer");
  const int unsigned long max_iter=cimg_option("-n",256,  "number of frames");
  const bool endian_swap=!cimg_option("--no-endian-swap",false,"do not swap endianess, by default it is done if needed (arch. dep.)");
  const bool verbose=cimg_option("--verbose",false,"Produce verbose output");
  const bool udp=!cimg_option("--simulation",false,"frame simulation, by default UDP frame are received");
  const bool debug=cimg_option("--debug",false,"debug output");

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


  //UDP related
  int udpSocket, nBytes=4;
  if(!udp){nBytes=width=4;}
  //! content buffer (as char)
  CImg<unsigned char> buffer(width);
  //! buffer as index (shared with buffer), i.e. cast to uint32, but still in net endian !
  CImg<unsigned int>  bindex(buffer.width()/4,buffer.height(),buffer.depth(),buffer.spectrum());
  const unsigned int* bindex_data=bindex._data;//keep memory of allocation place, before get shared data (for freeing)
  bindex._data=(unsigned int*)buffer.data();//share
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;

  //create UDP socket
  udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

  //configure settings in address struct
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(20485);
  serverAddr.sin_addr.s_addr = inet_addr("10.10.15.1");//"10.10.17.202");
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  //bind socket with address struct
  bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
  //initialize size variable to be used later on
  addr_size = sizeof serverStorage;

  //index
  unsigned int index=0;
  unsigned int prev_index=0;
  //index increment
  long inc=0;
  unsigned long count_drop=0;
  unsigned long count_drops=0;
  //loop index
  unsigned long i=0;
  if(verbose) if(!endian_swap) printf("information: NO swap endianess\n");
  if(verbose) printf("information: simulate UDP frame\n");
  if(verbose) printf("information: increment on first frame is supposed to be ok (not counted as a drop)\n");
  printf("#% 12s: % 11s % 10s % 11s; drop: % 12s      , % 12s","index iter.","frame hex","dec.","increment","drop","index drops");
//  while(1)
  for(;i<max_iter;++i)
  {
    if(udp)
    {//receiving UDP frame
      if(debug) printf("\ndebug: wait for UDP frame");
      //! receive any incoming UDP datagram. Address and port of requesting client will be stored on serverStorage variable
      nBytes = recvfrom(udpSocket,buffer.data(),buffer.width(),0,(struct sockaddr *)&serverStorage, &addr_size);
      //! \todo check nBytes buffer.width() ; add (lazy) resize ?
    }//UDP
    else
    {//draft simulation
      //! increment frame index (on first byte only), and simulate a frame drop at loop index 123 (note: looping over size of byte yield to -255 step)
      //simulation of value change
      buffer(0)=0x12;
      buffer(1)=0x34;
      buffer(2)=0x56;
      buffer(3)=0x78+(unsigned char)((i<123)?i:i+12);
      if(debug) buffer.print("buffer",false);
      if(debug) bindex.print("bindex",false);
    }//simulation
    //get frame index as first uint32 of buffer content
    {const unsigned int *b=(unsigned int *)buffer.data();index=(!endian_swap)?(*b):ntohl(*b);}//frame index (with endianess)
    //check increment
    inc=(long)index-(long)prev_index;
    if( (inc!=1) || debug)
    {
      //print loop index
      printf("\n#% 12ld: ",i);
      //print frame index as 4 bytes (as is in net buffer)
      for(unsigned int b=0;b<4;++b){unsigned char o=buffer(b); printf("%02x ",o);}
      //print frame index as uint32 (with endianess swap)
      printf("% 10u",index);
    }//drop|debug
    if(inc!=1)
    {//frame drop
      //drops
      if(i>0)
      {
        ++count_drop;
        count_drops+=abs(inc-1);//normal increment is 1
      }
      //print drop related
      printf(" % 11ld",inc);
      if(count_drop>0) printf("; drop: % 12lu drops, % 12lu index drops",count_drop,count_drops);
    }//drop
    //next loop
    prev_index=index;
  }//loop
  printf("\n");
  //summary of drops
  if(count_drops==0) printf("test pass: zero drop");
  else printf("test fail: in total, % 12ld drops, % 12ld index drops",count_drop,count_drops);
  printf(" on %d BoF (Bytes of Frame)",nBytes);
  if(nBytes==4) printf(" -warning: this might be a UDP simulation-");
  printf(".\n");
  //put back memory pointer (for freeing)
  bindex._data=(unsigned int*)bindex_data;
  return 0;
}//main

