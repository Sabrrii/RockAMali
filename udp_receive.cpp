//CoolImage
#include "CImg.h"
//C++ base
#include <iostream>
#include <string>
#include <vector>
//OpenMP
#include <omp.h>

//C base
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

// UDP point to point test

//! \todo drop of exactly 2^32 should not be taken into drops
//! \todo add NetCDF for storing both frame index and increment in loop (unlimited dim.)
//! \todo tests: ml507, RockAMali, numexo2

#define VERSION "v0.1.3g"

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

  unsigned int width=cimg_option("-s",1024, "size of UDP buffer [byte], might be schrunk (but not expanded !)");
  const int unsigned long max_iter=cimg_option("-n",256,  "number of frames");
  const bool endian_swap=!cimg_option("--no-endian-swap",false,"do not swap endianess, by default it is done if needed (arch. dep.)");
  const bool do_check_C=cimg_option("-C",false,NULL);//-C hidden option
        bool do_check=cimg_option("--do-check",do_check_C,"do data check, e.g. test pass (or -C option)");do_check=do_check_C|do_check;//same --do-check or -C option
  const bool verbose=cimg_option("--verbose",false,"Produce verbose output");
  const bool udp=!cimg_option("--simulation",false,"frame simulation, by default UDP frame are received");
  const bool debug=cimg_option("--debug",false,"debug output");
  const unsigned short port=cimg_option("-p",20485,"port where the packets are received");
  const std::string ip=cimg_option("-i", "10.10.17.202", "ip address of the sender");
  const int twait=cimg_option("-w", 3, "time out for receiving next frame [s]");
  const bool do_warmup_W=cimg_option("-W",false,NULL);//-W hidden option
  bool do_warmup=cimg_option("--do-warmup",do_warmup_W,"do data warmup, e.g. allocation and fill (or -W option)");do_warmup=do_warmup_W|do_warmup;//same --do-warmup or -W option

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

  //OpenMP locks
  omp_lock_t lock;omp_init_lock(&lock);
  unsigned int received=0; unsigned long current_i=0;bool done=false;

  #pragma omp parallel shared(lock, received,current_i,done)
  {
  int id=omp_get_thread_num(),tn=omp_get_num_threads();

  #pragma omp single
  {
  if(tn<2) {fprintf(stderr,"error: run error, this process need at least 2 threads (presently only %d available)\n",tn);exit(2);}
  else {printf("\ninfo: running %d threads\n",tn);fflush(stdout);}
  }//single

  //run threads
  switch(id)
  {
    case 0:
    {//watchdog
      unsigned int  count=0;
      unsigned long i=0;
      unsigned long t0=cimg::time();
      unsigned long t1;
      for(;;)
      {
        fflush(stdout);
        //locked section
        {
          omp_set_lock(&lock);
          //get currently received
          count=received;
          i=current_i;
          //reset received frame counter
          received=0;
          omp_unset_lock(&lock);
        }//lock
        //compute rate
        t1=t0;
        t0=cimg::time();
        const unsigned long dt=t0-t1;//delta time in ms
        const float rate=(count*width)/(1024.0*1024.0)/(float)(dt/1000.0);//MB/s
        if(i>0) fprintf(stderr,"\ninformation: i=%d, received=%d, dt=%dms, rate=%06.3fMB/s.",i,count,dt,rate);
        fflush(stderr);
        sleep(3);
        //! exit if work done
        //locked section
        {
          omp_set_lock(&lock);
          //exit
          if(done) break;
          omp_unset_lock(&lock);
        }//lock
      }//infinite loop
      break;
    }//watchdog
    case 1:
    {//receive

      //UDP related
      int udpSocket, nBytes=4;
      if(!udp){nBytes=width=4;}
      //! content buffer (as char)
      CImg<unsigned char> buffer(width);
      //! buffer as index (shared with buffer), i.e. cast to uint32, but still in net endian !
      CImg<unsigned int>  bindex((unsigned int*)buffer.data(),buffer.width()/4,buffer.height(),buffer.depth(),buffer.spectrum(),true);
      struct sockaddr_in receiverAddr;
      struct sockaddr_storage serverStorage;
      socklen_t addr_size;

      //create UDP socket
      udpSocket = socket(PF_INET, SOCK_DGRAM, 0);
      //timeout
      struct timeval tv;
      tv.tv_sec = twait;
      tv.tv_usec = 0;
      if (setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {fprintf(stderr,"error: while setting timeout to %d.\n",twait);exit(2);}

      //configure settings in address struct
      receiverAddr.sin_family = AF_INET;
      receiverAddr.sin_port = htons(port);
      receiverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
      memset(receiverAddr.sin_zero, '\0', sizeof receiverAddr.sin_zero);  

      //bind socket with address struct
      bind(udpSocket, (struct sockaddr *) &receiverAddr, sizeof(receiverAddr));
      //initialize size variable to be used later on
      addr_size = sizeof serverStorage;

      if(do_warmup) {printf("information: do warmup.\n");buffer.rand(0,255);bindex.max();}
      if(do_check) std::cout<<"information: checking data, i.e. test, activated (slow process !)\n";

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
    //while(1)
      for(;i<max_iter;++i)
      {
        if(udp)
        {//receiving UDP frame
          if(debug) printf("\ndebug: wait for UDP frame");
          //! receive any incoming UDP datagram. Address and port of requesting client will be stored on serverStorage variable
          //! - wait infinitely for first frame
          if(i==0)
          {
            while((nBytes=recvfrom(udpSocket,buffer.data(),buffer.width(),0,(struct sockaddr *)&serverStorage, &addr_size))<0)
            {
              fprintf(stderr,".");fflush(stderr);
            }//wait infinitely for first frame
          }//first frame
          else
          {//! timeout for others frames
            if((nBytes=recvfrom(udpSocket,buffer.data(),buffer.width(),0,(struct sockaddr *)&serverStorage, &addr_size))<0)
            {
              fprintf(stderr,"\nerror: receiving frame timeout %d s (i.e. -w option ; see recvfrom).\n",twait);fflush(stderr);
              break;
            }//timeout
          }//other frames
          //may resize (if width was bigger than first received frame)
          if(i==0) if(nBytes!=width)
          {
            //set new size for containers
            width=nBytes;
            fprintf(stderr,"\nwarning: resizing containers as received frame is %dBoF.\n",width);fflush(stderr);
            //resize
            buffer.assign(width);
            bindex.assign((unsigned int*)buffer.data(),buffer.width()/4,buffer.height(),buffer.depth(),buffer.spectrum(),true);//shared
          }//resize
          //!rate
          //locked section
          {
            omp_set_lock(&lock);
            //increment received frame counter
            ++received;
            current_i=i;
            omp_unset_lock(&lock);
          }//lock
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
        //! get frame index as first uint32 of buffer content
        {const unsigned int *b=(unsigned int *)buffer.data();index=(!endian_swap)?(*b):ntohl(*b);}//frame index (with endianess)
        //! check increment
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
        //! check full content
        if(do_check)
        {
          const unsigned int sindex=(!endian_swap)?(index):ntohl(index);
          if(bindex!=sindex)
          {
            std::cout<<"check: full content check failed for #"<<index<<", i.e. "<<sindex<<"with net endian."<<std::endl;
            if(debug) bindex.print("b index");
          }
        }//check
        //next loop
        prev_index=index;
      }//loop
      printf("\n");
      //summary of drops
      if(count_drops==0) printf("test pass: zero drop");
      else printf("test fail: in total, % 12ld drops, % 12ld index drops (i.e. %6.4f%%)",count_drop,count_drops,count_drops*100.0/(float)max_iter);
      printf(" on %d BoF (Bytes of Frame)",width);
      if(nBytes==4) printf(" -warning: this might be a UDP simulation-");
      printf(".\n");
      //! work done exiting
      //locked section
      {
        omp_set_lock(&lock);
        //work done (for all threads)
        done=true;
        omp_unset_lock(&lock);
      }//lock
      break;
    }//receive
  }//switch(id)
  }//parallel section
  return 0;
}//main

