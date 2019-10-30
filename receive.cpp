//CoolImage
#include "CImg.h"

//C++ base
#include <iostream>
#include <string>
#include <vector>

//OpenMP
#include <omp.h>

#define VERSION "v0.5.0e"

//thread lock
#include "CDataStore.hpp"
#ifdef DO_GPU
#include "CDataProcessorGPUfactory.hpp"
#endif //DO_GPU
#include "CDataProcessor.hpp"
#include "CDataReceive.hpp"

using namespace cimg_library;
using boost::asio::ip::udp;

//types
typedef unsigned char  Taccess;
typedef unsigned short Tdata;

int main(int argc,char **argv)
{
  ///command arguments, i.e. CLI option
  cimg_usage(std::string("receive (,  process) and store data.\n" \
  " It uses different GNU libraries (see --info option)\n\n" \
  " usage: ./store -h -I\n" \
  "        ./store -s 1024 -n 123 -X true -o samples/sample.png -p 1234\n" \
  "\n version: "+std::string(VERSION)+"\n compilation date:" \
  ).c_str());//cimg_usage

  const char* imagefilename = cimg_option("-o","samples/sample.png","output file name (e.g. \"-o data.png -d 3\" gives data_???.png)");
  const char* resultfilename =cimg_option("-r","results/sample.png","result file name (e.g. \"-o proc.png -d 3\" gives proc_???.png)");
  const unsigned int digit=cimg_option("-d",6,  "number of digit for file names");
  const int width=cimg_option("-s",1024, "size   of udp buffer");
  const int count=cimg_option("-n",123,  "number of vector");  //! \todo [high] should be NO count of the vectors with an infinite loop (to implement)
  const int nbuffer=cimg_option("-b",12, "size   of vector buffer (total size is b*s*4 Bytes)");
  const int threadCount=cimg_option("-c",0,"thread count (2 for receiving only and 4 for processing)");
  const unsigned short port=cimg_option("-p", 1234, "port where the packets are sent on the receiving device");
#ifdef DO_GPU
  const bool use_GPU_G=cimg_option("-G",false,NULL);//-G hidden option
        bool use_GPU=cimg_option("--use-GPU",use_GPU_G,"show GUI (or -G option)");use_GPU=use_GPU_G|use_GPU;//same --use-GPU or -G option
  const std::string processing_type=cimg_option("--GPU-factory","program","GPU processing type, e.g. program or function");
  //show type list in factory
  std::vector<std::string> type_list;CDataProcessorGPUfactory<Tdata, Taccess>::show_factory_types(type_list);std::cout<<std::endl;
#endif //DO_GPU
  const bool do_check_C=cimg_option("-C",false,NULL);//-C hidden option
        bool do_check=cimg_option("--do-check",do_check_C,"do data check, e.g. test pass (or -C option)");do_check=do_check_C|do_check;//same --do-check or -C option
  const bool do_check_CE=cimg_option("-E",false,NULL);//-E hidden option
        bool do_check_exit=cimg_option("--do-check-exit",do_check_CE,"do data check exit on first error (or -E option)");do_check_exit=do_check_CE|do_check_exit;//same --do-check_exit or -E option
  const bool do_warmup_W=cimg_option("-W",false,NULL);//-W hidden option
        bool do_warmup=cimg_option("--do-warmup",do_check_C,"do data warmup, e.g. allocation and fill (or -W option)");do_warmup=do_warmup_W|do_warmup;//same --do-warmup or -W option

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
  //! result circular buffer
  CImgList<Tdata> results(nbuffer,width,1,1,1);
  results[0].fill(0);
  results[0].print("result",false);
  //accessR locking
  omp_lock_t lckR;omp_init_lock(&lckR);

  //! access and status of buffer
  CImg<Taccess> access(nbuffer,1,1,1);
  access.fill(0);//free
  access.print("access (free state)",false);fflush(stderr);
  //! access and status of Result buffer
  CImg<Taccess> accessR(nbuffer,1,1,1);
  accessR.fill(0);//free
  accessR.print("accessR (free state)",false);fflush(stderr);

  //! receive data
  std::vector<omp_lock_t*> locks;locks.push_back(&print_lock);locks.push_back(&lck);locks.push_back(&lckR);
  std::vector<omp_lock_t*> locksR;locksR.push_back(&print_lock);locksR.push_back(&lckR);

  if(do_warmup)
  {
    std::cout<<"information: warming up";
    std::cout<<" ~"<<2*nbuffer*width*sizeof(Tdata)/1024/1024<<" MB of RAM ";
    cimglist_for(images,i)  images[i].fill(nbuffer-i);
    std::cout<<"v";
    cimglist_for(results,i) results[i].fill(nbuffer-i);
    std::cout<<"v\n";
  }

#ifdef DO_GPU
  //Choosing the target for OpenCL computing
  boost::compute::device gpu = boost::compute::system::default_device();
  #pragma omp parallel shared(print_lock, access,images, accessR,results, gpu)
#else
  #pragma omp parallel shared(print_lock, access,images, accessR,results)
#endif //!DO_GPU
  {
  int id=omp_get_thread_num(),tn=omp_get_num_threads();

  #pragma omp single
  {
  if(tn<4) {if(tn!=2) {printf("error: run error, this process need at least 2 (or 4) threads (presently only %d available)\n",tn);exit(2);}}
  else {printf("info: running %d threads\n",tn);fflush(stdout);}
  if(do_check) std::cout<<"information: checking data, i.e. test, activated (slow process !)\n";
  }//single

  switch(id)
  {
    case 0:
    {//receive
      CDataReceive<Tdata, Taccess> receive(locks, port, width, &io_service, do_check,do_check_exit);
      receive.run(access,images, count);
      receive.show_checking();
      break;
    }//receive
    case 1:
    {//store
      //! store either if processing or not (i.e. 2 or 4 threads)
      if(tn==2) std::cout<<"information: no processing as thread number need to be at least up to 4."<<std::endl<<std::flush;
      CDataStore<Tdata, Taccess> store(locks, imagefilename,digit, (tn==2)?CDataAccess::STATUS_FILLED:CDataAccess::STATUS_PROCESSED);//images
      store.run(access,images, count);
      break;
    }//store
    case 2:
    {//process
#ifdef DO_GPU
      if(use_GPU)
      {//GPU
      std::cout<<"information: use GPU for processing."<<std::endl<<std::flush;
//      CDataProcessorGPU<Tdata, Taccess> *process(
      CDataProcessorGPU<Tdata, Taccess> *process=CDataProcessorGPUfactory<Tdata, Taccess>::NewCDataProcessorGPU(processing_type,type_list
      , locks, gpu,width
      , CDataAccess::STATUS_RECEIVED,CDataAccess::STATUS_PROCESSED //images
      , CDataAccess::STATUS_FREE,    CDataAccess::STATUS_FILLED    //results
      , do_check
      );
      std::cout<<"information: processing type is the one in "<<process->class_name<<" class."<<std::endl<<std::flush;
      process->run(access,images, accessR,results, count);
      }//GPU
      else
#endif
      {//CPU
      std::cout<<"information: use CPU for processing."<<std::endl<<std::flush;
      CDataProcessor<Tdata,Taccess> process(locks
      , CDataAccess::STATUS_RECEIVED,CDataAccess::STATUS_PROCESSED //images
      , CDataAccess::STATUS_FREE,    CDataAccess::STATUS_FILLED    //results
      , do_check
      );
      process.run(access,images, accessR,results, count);
      }//CPU
      break;
    }//process
    case 3:
    {//store
      CDataStore<Tdata, Taccess> store(locksR, resultfilename,digit, CDataAccess::STATUS_FILLED);//results
      store.run(accessR,results, count);
      break;
    }//store
  }//switch(id)
  }//parallel section

  return 0;
}//main

