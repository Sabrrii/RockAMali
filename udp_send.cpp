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

#ifdef DO_NETCDF
#include "CImg_NetCDF.h"
#endif //DO_NETCDF

// UDP point to point test

#define VERSION "v0.1.5v"

using namespace cimg_library;

//! CLI option parse and ...
int main(int argc, char **argv)
{
  ///command arguments, i.e. CLI option
  cimg_usage(std::string("send UDP frame with increment (i.e. index) as first uint32 of content, the rest is either with random content or this index.\n" \
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
  const unsigned long max_iter=cimg_option("-n",256,  "number of frames");
  const bool endian_swap=!cimg_option("--no-endian-swap",false,"do not swap endianess, by default it is done if needed (arch. dep.)");
  const bool do_fill_F=cimg_option("-C",false,NULL);//-C hidden option
        bool do_fill=cimg_option("--do-fill",do_fill_F,"do fill entire frame with index -this is slow process- (or -F option) otherwise single random content frame is sent except first uint32 that get current index");do_fill=do_fill_F|do_fill;//same --do-fill or -F option
  const bool verbose=cimg_option("--verbose",false,"Produce verbose output");
  const bool debug=cimg_option("--debug",false,"debug output");
  const unsigned short port=cimg_option("-p",20485,"port where the packets are send on the receiving device");
  const std::string ip=cimg_option("-i", "10.10.17.202", "ip address of the receiver");
  const int  twait=cimg_option("-w", 123, "waiting time between udp frames [us] (mean rate if dw>0)");
  const int dtwait=cimg_option("-dw",  0, "waiting time range between udp frames [us]");
  const bool do_rnd_wait=(dtwait!=0);
  const int dtw_sz=cimg_option("-dws",(int)max_iter, "size of random waiting times between udp frames [us]");
  const bool do_warmup_W=cimg_option("-W",false,NULL);//-W hidden option
  bool do_warmup=cimg_option("--do-warmup",do_warmup_W,"do data warmup, e.g. allocation and fill (or -W option)");do_warmup=do_warmup_W|do_warmup;//same --do-warmup or -W option
  const bool do_ramp_R=cimg_option("-R",false,NULL);//-R hidden option
  bool do_ramp=cimg_option("--do-ramp",do_ramp_R,"do rate ramp on first 256 frames, e.g. ethernet rate raise to requested (or -R option)");do_ramp=do_ramp_R|do_ramp;//same --do-ramp or -R option
  const int ramp_width=cimg_option("-r",256,"ramp width");
#ifdef DO_NETCDF
  const std::string file_name=cimg_option("-o","udp_send.nc","output file name (e.g. -o data.nc)");//ouput file name for a few parameters, especially random waits
#endif //NetCDF

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
//    std::cout<<"  CParameterNetCDF."<<CDL_PARAMETER_VERSION<<std::endl;
//    std::cout<<"  NcTypeInfo."<<NETCDF_TYPE_INFO_VERSION;
#endif //NetCDF
    std::cout<<std::endl;return 0;
  }//same --version or -v option

  if(show_help) {/*print_help(std::cerr);*/return 0;}
  //}CLI option

  //! content buffer (as char)
  CImg<unsigned char> buffer(width);
  //! buffer as index (shared with buffer), i.e. cast to uint32, but still in net endian !
  CImg<unsigned int>  bindex((unsigned int*)buffer.data(),buffer.width()/4,buffer.height(),buffer.depth(),buffer.spectrum(),true);//share

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

  CImg<unsigned int> twaits;//wait times for ramp and random waiting
  //assign and constant init
  if(do_ramp){if(do_rnd_wait) twaits.assign(dtw_sz); else {twaits.assign(ramp_width+1);twaits=twait;}}
  else       {if(do_rnd_wait) twaits.assign(dtw_sz); else {twaits.assign(1);twaits(0)=twait;}}
  //init random
  if(do_rnd_wait){twaits.rand(twait-dtwait/2,twait+dtwait/2);twaits.print("random wait");fflush(stderr);}
  //init ramp
  if(do_ramp) {cimg_for_inX(twaits,0,ramp_width-1,x) {const int tw=twait*(ramp_width-x);twaits(x)=tw;}}
#ifdef DO_NETCDF
  //NetCDF format
  CImgNetCDF<int> nc;
  CImg<int> nc_img(1);//temporary image for type conversion
  //dimension names
  std::vector<std::string> dim_names;
  std::string dim_time;
  //variable names (and its unit)
  std::string var_name;
  std::string unit_name;
  dim_time="dimF";
  dim_names.push_back("dimS");
  var_name="send_wait";
  unit_name="us";
  //open file
std::cout << "CImgNetCDF::saveNetCDFFile(" << file_name << ",...) return " << nc.saveNetCDFFile((char*)file_name.c_str()) << std::endl;
  //add global attributes
  nc.pNCFile->add_att("library","CImg_NetCDF");
  nc.pNCFile->add_att("library_version",CIMG_NETCDF_VERSION);
  //declare dims and vars
std::cout << "CImgNetCDF::addNetCDFDims(" << file_name << ",...) return " << nc.addNetCDFDims(nc_img,dim_names,dim_time) << std::endl<<std::flush;
std::cout << "CImgNetCDF::addNetCDFVar(" << file_name << ",...) return " << nc.addNetCDFVar(nc_img,var_name,unit_name) << std::endl<<std::flush;
  //add attributes
  if (!(nc.pNCvar->add_att("long_name","wait time between sending UDP frames"))) std::cerr<<"error: while adding attribute long name (NC_ERROR)."<<std::endl;
  if (!(nc.pNCvar->add_att("wait_unit","us"))) std::cerr<<"error: while adding attribute wait unit (NC_ERROR)."<<std::endl;
  if(dtwait==0)
  {
    if (!(nc.pNCvar->add_att("wait",twait))) std::cerr<<"error: while adding attribute wait (NC_ERROR)."<<std::endl;
  }
  else
  {
    if (!(nc.pNCvar->add_att("wait_average",twait))) std::cerr<<"error: while adding attribute wait average (NC_ERROR)."<<std::endl;
    if (!(nc.pNCvar->add_att("wait_delta",dtwait))) std::cerr<<"error: while adding attribute wait delta (NC_ERROR)."<<std::endl;
    if (!(nc.pNCvar->add_att("wait_min",(int)twaits.min()))) std::cerr<<"error: while adding attribute wait delta (NC_ERROR)."<<std::endl;
    if (!(nc.pNCvar->add_att("wait_max",(int)twaits.max()))) std::cerr<<"error: while adding attribute wait delta (NC_ERROR)."<<std::endl;
    if(do_ramp)
    {//cut ramp part
      CImg<int> ctwaits(&(twaits(ramp_width-1)),twaits.width()-ramp_width,1,1, true);//shared
      if (!(nc.pNCvar->add_att("wait_max_without_ramp",(int)ctwaits.max()))) std::cerr<<"error: while adding attribute wait delta (NC_ERROR)."<<std::endl;
    }//cut ramp part
  }//wait as attribute
  if(do_ramp==0)
  {
    if (!(nc.pNCvar->add_att("ramp","disable"))) std::cerr<<"error: while adding attribute ramp (NC_ERROR)."<<std::endl;
  }
  else
  {
    if (!(nc.pNCvar->add_att("ramp","enable")))  std::cerr<<"error: while adding attribute ramp (NC_ERROR)."<<std::endl;
    if (!(nc.pNCvar->add_att("ramp_width",(int)ramp_width))) std::cerr<<"error: while adding attribute ramp size  (NC_ERROR)."<<std::endl;
  }//ramp as attribute

  if (!(nc.pNCvar->add_att("frame_size_unit","Byte"))) std::cerr<<"error: while adding attribute frame size  (NC_ERROR)."<<std::endl;
  if (!(nc.pNCvar->add_att("frame_size",(int)width))) std::cerr<<"error: while adding attribute frame size  (NC_ERROR)."<<std::endl;
  //add data
  std::cout << "CImgNetCDF::addNetCDFData(" << file_name << ",...)"<< std::endl<<std::flush;
  if(!debug)
  {
    cimg_forX(twaits,x)
    {
      nc_img(0)=twaits(x);
      nc.addNetCDFData(nc_img);
    }//for loop
  }//debug
#endif //NetCDF

//  while(1)
  for(int i=0,itw=0;i<max_iter;++i,++itw)
  {
    if(debug) printf("\n");
    printf("i=%d\r",i);if(i==0) fflush(stdout);
    //update loop index in UDP buffer
    if(do_fill) bindex=(!endian_swap)?i:ntohl(i);//slow
    else     bindex(0)=(!endian_swap)?i:ntohl(i);//fast
    //send buffer to receiver
    sendto(clientSocket,buffer,width,0,(struct sockaddr *)&receiverAddr,addr_size);
    //control data rate
    if(itw==twaits.width()) itw=0;
#ifdef DO_NETCDF
    if(debug)
    {
      nc_img(0)=twaits(itw);
      //specific default wait
      if(ramp_width<i) if(do_ramp) if(!do_rnd_wait) nc_img(0)=twaits(twaits.width()-1);
      nc.addNetCDFData(nc_img);
    }//debug
#endif //NetCDF
    if(do_ramp) if(i<ramp_width) {const int tw=twaits(i);if(verbose) printf("wait=%dus\n",tw);usleep(tw);continue;}
    if(do_rnd_wait) {if(debug) printf("\nwait=%dus",twaits(itw));usleep(twaits(itw));continue;}
    if(debug) printf("\nwait=%dus",twait);
    usleep(twait);
  }//for loop
  printf("\n");
  return 0;
}//main

