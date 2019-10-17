//CoolImage
#include "CImg.h"

//C++ base
#include <iostream>
#include <string>
#include <vector>

//OpenMP
#include <omp.h>

#define VERSION "v0.4.1d"

//thread lock
#include "CDataGenerator.hpp"
#include "CDataProcessor_morphomath.hpp"
#ifdef DO_GPU
#include "CDataProcessorGPU.hpp"
#endif
#include "CDataStore.hpp"

using namespace cimg_library;

#define S 0 //sample

//types
typedef unsigned char Taccess;
typedef unsigned int  Tdata;

int main(int argc,char **argv)
{
  ///command arguments, i.e. CLI option
  cimg_usage(std::string("generate, process and store data.\n" \
  " It uses different GNU libraries (see --info option)\n\n" \
  " usage: ./process -h\n" \
  "        ./process -s 1024 -n 123 -X true -p 1234 -i 10.10.15.1 -w 1234657\n" \
  "\n version: "+std::string(VERSION)+"\n compilation date:" \
  ).c_str());//cimg_usage

  const char* imagefilename = cimg_option("-o","sample.cimg","output file name (e.g. \"-o data.cimg -d 3\" gives data_???.cimg)");
  const int digit=cimg_option("-d",6,  "number of digit for file names");
  const int width=cimg_option("-s",1024, "size   of udp buffer");
  const int count=cimg_option("-n",256,  "number of frames");
  const int nbuffer=cimg_option("-b",12, "size   of vector buffer (total size is b*s*4 Bytes)");
  const int threadCount=cimg_option("-c",3,"thread count (threads above 2 are processing one)");
#ifdef DO_GPU
  const bool use_GPU_G=cimg_option("-G",false,NULL);//-G hidden option
        bool use_GPU=cimg_option("--use-GPU",use_GPU_G,"use GPU for compution (or -G option)");use_GPU=use_GPU_G|use_GPU;//same --use-GPU or -G option
#endif //DO_GPU
  const bool do_check_C=cimg_option("-C",false,NULL);//-G hidden option
        bool do_check=cimg_option("--do-check",do_check_C,"do data check, e.g. test pass (or -C option)");do_check=do_check_C|do_check;//same --do_check or -C option

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

  //! thread locks
  std::vector<omp_lock_t*> locks;locks.push_back(&print_lock);locks.push_back(&lck);locks.push_back(&lckR);
  std::vector<omp_lock_t*> locksR;locksR.push_back(&print_lock);locksR.push_back(&lckR);

#ifdef DO_GPU
  //Choosing the target for OpenCL computing
  boost::compute::device gpu = boost::compute::system::default_device();
  #pragma omp parallel shared(print_lock, access,images, accessR,results, gpu)
#else
  #pragma omp parallel shared(print_lock, access,images, accessR,results)
#endif //!DO_GPU
  {
  int id=omp_get_thread_num(),tn=omp_get_num_threads();
  unsigned int stride=tn-2;
  unsigned int start=0;

  #pragma omp single
  {
  if(tn<3) {printf("error: run error, this process need at least 3 threads (presently only %d available)\n",tn);exit(2);}
  else {printf("\ninfo: running %d threads\n",tn);fflush(stdout);}
  if(do_check) std::cout<<"information: checking data, i.e. test, activated (slow process !)\n";
  }//single

  //run threads
  switch(id)
  {
    case 0:
    {//generate
      CDataGenerator<Tdata,Taccess> generate(locks);
      generate.run(access,images, count);
      break;
    }//generate
    case 1:
    {//store
      CDataStore<Tdata,Taccess> store(locksR, imagefilename,digit, CDataAccess::STATUS_FILLED);
      store.run(accessR,results, count);
      break;
    }//store
    default:
    {//process
      start=id-2;//e.g. #3 -> 1
#ifdef DO_GPU
      if(use_GPU)
      {//GPU
      std::cout<<"information: use GPU for processing (from "<<start<<" by step of "<<stride<<")."<<std::endl<<std::flush;
//      CDataProcessorGPU<Tdata, Taccess> process(locks, gpu,width
//      CDataProcessorGPU_vPvMv_lambda<Tdata, Taccess> process(locks, gpu,width
      CDataProcessorGPU_lambda<Tdata, Taccess> process(locks, gpu,width
      , CDataAccess::STATUS_FILLED, CDataAccess::STATUS_FREE  //images
      , CDataAccess::STATUS_FREE,   CDataAccess::STATUS_FILLED//results
      , do_check
      );
      process.run(access,images, accessR,results, count, stride,start);
      process.show_checking();
      }//GPU
      else
#endif
      {//CPU
      std::cout<<"information: use CPU for processing (from "<<start<<" by step of "<<stride<<"."<<std::endl<<std::flush;
//      CDataProcessor<Tdata,Taccess> process(locks
//      CDataProcessor_vPvMv<Tdata,Taccess> process(locks
      CDataProcessor_kernel<Tdata,Taccess> process(locks
      , CDataAccess::STATUS_FILLED, CDataAccess::STATUS_FREE  //images
      , CDataAccess::STATUS_FREE,   CDataAccess::STATUS_FILLED//results
      , do_check
      );
      process.run(access,images, accessR,results, count, stride,start);
      process.show_checking();
      }//CPU
      break;
    }//process
  }//switch(id)
  }//parallel section

  access.print("access (free state)",false);fflush(stderr);
  images.print("CImgList",false);

  accessR.print("accessR (free state)",false);fflush(stderr);
  results.print("CImgListR",false);

  return 0;
}//main

