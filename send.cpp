//CoolImage
#include "CImg.h"

//C++ base
#include <iostream>
#include <string>
#include <vector>

//OpenMP
#include <omp.h>

#define VERSION "v0.5.7r"

//thread lock
#include "CDataGenerator_factory.hpp"
#include "CDataSend.hpp"

using namespace cimg_library;

#define S 0 //sample

//types
typedef unsigned char Taccess;
typedef unsigned int  Tdata;

int main(int argc,char **argv)
{
  ///command arguments, i.e. CLI option
  cimg_usage(std::string("generate and send data via UDP.\n" \
  " It uses different GNU libraries (see --info option)\n\n" \
  " usage: ./send -h\n" \
  "        ./send -s 1024 -n 123 -X true -p 1234 -i 10.10.15.1 -w 1234657\n" \
  "\n version: "+std::string(VERSION)+"\n compilation date:" \
  ).c_str());//cimg_usage

  //const char* imagefilename = cimg_option("-o","sample.cimg","output file name (e.g. \"-o data.cimg -d 3\" gives data_???.cimg)");
  //const int digit=cimg_option("-d",6,  "number of digit for file names");
  const int width=cimg_option("-s",1024, "size   of udp buffer");
  const int count=cimg_option("-n",256,  "number of frames");
  const int nbuffer=cimg_option("-b",12, "size   of vector buffer (total size is b*s*4 Bytes)");
  const int threadCount=cimg_option("-c",2,"thread count");
  const unsigned short port=cimg_option("-p",1234,"port where the packets are send on the receiving device");
  const std::string ip=cimg_option("-i", "10.10.15.1", "ip address of the receiver");
  const int twait=cimg_option("-w", 123456789, "waiting time between udp frames");
//! generator factory
  const std::string generator_type=cimg_option("--generator-factory","count","generator type, e.g. count, random or peak");
  //show type list in generator factory
  std::vector<std::string> generator_type_list;CDataGenerator_factory<Tdata, Taccess>::show_factory_types(generator_type_list);std::cout<<std::endl;

  //conversion of twait into a boost::uint64_t
  const boost::uint64_t wait=static_cast<std::size_t>(twait);

  ///standard options
  #if cimg_display!=0
  const bool show_X=cimg_option("-X",true,NULL);//-X hidden option
  bool show=cimg_option("--show",show_X,"show GUI (or -X option)");show=show_X|show;//same --show or -X option
  #endif
  const bool show_h   =cimg_option("-h",    false,NULL);//-h hidden option
        bool show_help=cimg_option("--help",show_h,"help (or -h option)");show_help=show_h|show_help; //same --help or -h option
  bool show_info=cimg_option("-I",false,NULL);//-I hidden option
  if( cimg_option("--info",show_info,"show compilation options (or -I option)") ) {show_info=true;cimg_library::cimg::info();}//same --info or -I option
  bool show_version=cimg_option("-v",false,NULL);//-v hidden option
  if( cimg_option("--version",show_version,"show version (or -v option)") ) {show_version=true;std::cout<<VERSION<<std::endl;return 0;}//same --version or -v option
  if(show_help) {/*print_help(std::cerr);*/return 0;}
  //}CLI option

  //OpenMP
  if(threadCount>0)
  {//user number of thread
    omp_set_dynamic(0);
    omp_set_num_threads(threadCount);
  }//user

  //OpenMP locks
  omp_lock_t print_lock;omp_init_lock(&print_lock);

  //! circular buffer
  CImgList<Tdata> images(nbuffer,width,1,1,1);
  images[0].fill(0);
  images[0].print("image",false);
  //access locking
  omp_lock_t lck;omp_init_lock(&lck);

  //! access and status of buffer
  CImg<Taccess> access(nbuffer,1,1,1);
  access.fill(0);//free
  access.print("access (free state)",false);fflush(stderr);

  //! thread locks
  std::vector<omp_lock_t*> locks;locks.push_back(&print_lock);locks.push_back(&lck);

  #pragma omp parallel shared(print_lock, access,images)
  {
  int id=omp_get_thread_num(),tn=omp_get_num_threads();

  #pragma omp single
  {
  if(tn<2) {printf("error: run error, this process need at least 2 threads (presently only %d available)\n",tn);exit(2);}
  else {printf("\ninfo: running %d threads\n",tn);fflush(stdout);}
  }//single

  //run threads
  switch(id)
  {
    case 0:
    {//generate
      CDataGenerator<Tdata, Taccess> *generate=CDataGenerator_factory<Tdata, Taccess>::NewCDataGenerator(generator_type,generator_type_list
      , locks
      );
      std::cout<<"information: generator type is the one in "<<generate->class_name<<" class."<<std::endl<<std::flush;
      generate->run(access,images, count);
      break;
    }//generate
    case 1:
    {//send
      CDataSend<Tdata,Taccess> send(locks,ip,port,wait);
      send.run(access,images, count);
      break;
    }//send
  }//switch(id)
  }//parallel section

  access.print("access (free state)",false);fflush(stderr);
  images.print("CImgList",false);
  return 0;
}//main

