//CoolImage
#include "CImg.h"

//C++ base
#include <iostream>
#include <string>
#include <vector>

//OpenMP
#include <omp.h>

#define VERSION "v0.4.5e"

//thread lock
#include "CDataGenerator.hpp"
#include "CDataProcessor_morphomath.hpp"
#ifdef DO_GPU
#ifdef DO_GPU_NO_QUEUE
#warning "DO_GPU_NO_QUEUE active (this must be CODE TEST only)"
#include "CDataProcessorGPU.hpp"
#else //DO_GPU_NO_QUEUE
#ifdef DO_GPU_SEQ_QUEUE
#warning "DO_GPU_SEQ_QUEUE active (this must be CODE TEST only)"
#endif
#include "CDataProcessorGPUqueue.hpp"
#endif //with queue
#endif //DO_GPU
#include "CDataStore.hpp"

using namespace cimg_library;

#define S 0 //sample

//types
typedef unsigned char Taccess;
typedef unsigned int  Tdata;

int main(int argc,char **argv)
{
  ///command arguments, i.e. CLI option
  cimg_usage(std::string("generate, process and store data sequentialy.\n" \
  " It uses different GNU libraries (see --info option)\n\n" \
  " usage: ./process -h\n" \
  "        ./process -s 1024 -n 123 -X true -p 1234 -i 10.10.15.1 -w 1234657\n" \
  "\n version: "+std::string(VERSION)+"\n compilation date:" \
  ).c_str());//cimg_usage

  const char* imagefilename = cimg_option("-o","sample.cimg","output file name (e.g. \"-o data.cimg -d 3\" gives data_???.cimg)");
  const int digit=cimg_option("-d",6,  "number of digit for file names");
  const int width=cimg_option("-s",1024, "size   of udp buffer");
  const int count=cimg_option("-n",256,  "number of frames");
  const int nbuffer=1;
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
  {//user number of thread
    omp_set_dynamic(0);
    omp_set_num_threads(1);
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

  //do check
  unsigned int check_error=0;
  //info
  std::string   process_class_name;
  std::string deprocess_class_name;

#ifdef DO_GPU
  //Choosing the target for OpenCL computing
  boost::compute::device gpu = boost::compute::system::default_device();
  CImgList<Tdata> limages(nbuffer,width,1,1,1);
  std::vector<compute::future<void>  > waits(nbuffer);//this may be filled in kernel
  compute::vector<Tdata> *device_vector1;compute::vector<Tdata> *device_vector3;//need more in process
  #pragma omp parallel shared(print_lock, access,images, accessR,results, check_error, gpu,limages,waits,device_vector1,device_vector3)
#else
  #pragma omp parallel shared(print_lock, access,images, accessR,results, check_error)
#endif //!DO_GPU
  {
  int id=omp_get_thread_num(),tn=omp_get_num_threads();

  #pragma omp single
  {
  printf("\ninfo: running single thread over %d.\n",tn);
  if(do_check) std::cout<<"information: checking data, i.e. test, activated (slow process !)\n";
  fflush(stdout);
  }//single

  //run threads
  switch(id)
  {
    case 0:
    {//sequential
     //generate
      CDataGenerator<Tdata,Taccess> generate(locks);
     //process
      CDataProcessor<Tdata,Taccess> *process;
      CDataProcessor<Tdata,Taccess> *deprocess;
#ifdef DO_GPU
      CImgList<Tdata> limages(nbuffer,width,1,1,1);
      if(use_GPU)
      {//GPU
     #ifdef DO_GPU_NO_QUEUE
      std::cout<<"information: use GPU for processing."<<std::endl<<std::flush;
//      process=new CDataProcessorGPU<Tdata, Taccess>(locks, gpu,width
//      process=new CDataProcessorGPU_vPvMv_lambda<Tdata, Taccess>(locks, gpu,width
//      process=new CDataProcessorGPU_lambda<Tdata, Taccess>(locks, gpu,width
//      process=new CDataProcessorGPU_closure<Tdata, Taccess>(locks, gpu,width
//      process=new CDataProcessorGPU_function_lambda<Tdata, Taccess>(locks, gpu,width
      process=new CDataProcessorGPU_function<Tdata, Taccess>(locks, gpu,width
//      process=new CDataProcessorGPU_function_macro<Tdata, Taccess>(locks, gpu,width
      , CDataAccess::STATUS_FILLED, CDataAccess::STATUS_FREE  //images
      , CDataAccess::STATUS_FREE,   CDataAccess::STATUS_FILLED//results
      , do_check
      );
     #else //DO_GPU_NO_QUEUE
     #ifdef  DO_GPU_SEQ_QUEUE
      std::cout<<"information: use GPU for processing (sequential queue)."<<std::endl<<std::flush;
      process=new CDataProcessorGPUqueue<Tdata, Taccess>(locks, gpu,width
      , limages, waits[0],device_vector1,device_vector3
      , CDataAccess::STATUS_FILLED, CDataAccess::STATUS_FREE  //images
      , CDataAccess::STATUS_FREE,   CDataAccess::STATUS_FILLED//results
      , do_check
      );
     #else //!DO_GPU_SEQ_QUEUE
      std::cout<<"information: use GPU for processing (enqueue and dequeue)."<<std::endl<<std::flush;
      process=new CDataProcessorGPUenqueue<Tdata, Taccess>(locks, gpu,width
      , limages, waits[0],device_vector1,device_vector3
      , CDataAccess::STATUS_FILLED, CDataAccess::STATUS_FREE  //images
      , CDataAccess::STATUS_FREE,   CDataAccess::STATUS_FILLED//results
      , do_check
      );
      deprocess=new CDataProcessorGPUdequeue<Tdata, Taccess>(locks, gpu,width
      , limages, waits[0],device_vector1,device_vector3
      , CDataAccess::STATUS_FILLED, CDataAccess::STATUS_FREE  //images
      , CDataAccess::STATUS_FREE,   CDataAccess::STATUS_FILLED//results
      , do_check
      );
      deprocess_class_name=deprocess->class_name;
     #endif //!DO_GPU_SEQ_QUEUE
     #endif //!DO_GPU_NO_QUEUE
      }//GPU
      else
#endif //DO_GPU
      {//CPU
      std::cout<<"information: use CPU for processing."<<std::endl<<std::flush;
//      process=new CDataProcessor<Tdata, Taccess>(locks
//      process=new CDataProcessor_vPvMv<Tdata, Taccess>(locks
      process=new CDataProcessor_kernel<Tdata, Taccess>(locks
      , CDataAccess::STATUS_FILLED, CDataAccess::STATUS_FREE  //images
      , CDataAccess::STATUS_FREE,   CDataAccess::STATUS_FILLED//results
      , do_check
      );
      }//CPU
      process_class_name=process->class_name;
     //store
      CDataStore<Tdata,Taccess> store(locksR, imagefilename,digit, CDataAccess::STATUS_FILLED);
      //run
      for(unsigned int i=0;i<count;++i)
      {
        generate.iteration(access,images,0,i);
        process->iteration(access,images, accessR,results, 0,i);
#ifdef DO_GPU
       #ifndef DO_GPU_NO_QUEUE
       #ifndef DO_GPU_SEQ_QUEUE
        deprocess->iteration(access,images, accessR,results, 0,i);
       #endif //!DO_GPU_SEQ_QUEUE
       #endif //!DO_GPU_NO_QUEUE
#endif
        store.iteration(accessR,results, 0,i);
        //check
        if(do_check)
        {
          if(images[0] ==i) NULL; else {++check_error;std::cout<<"compution error: bad main generate class for this test."<<std::endl<<std::flush;}
          //if(results[0]==i+i*i) NULL; else {++check_error;std::cout<<"compution error: bad main check (i.e. test failed) on iteration #"<<i<<" (value="<<results[0](0)<<")."<<std::endl<<std::flush;}
         process->show_checking();
        }
      }//vector loop
      break;
    }//sequential
  }//switch(id)
  }//parallel section

  access.print("access (free state)",false);fflush(stderr);
  images.print("CImgList",false);

  accessR.print("accessR (free state)",false);fflush(stderr);
  results.print("CImgListR",false);

  if(do_check)
  {
    if(check_error>0) std::cout<<"test: fail ("<<check_error<<" errors over "<<count<<" iterations)";
    else std::cout<<"test: pass";
    std::cout<<" (with "<<process_class_name;
#ifdef DO_GPU
       #ifndef DO_GPU_NO_QUEUE
       #ifndef DO_GPU_SEQ_QUEUE
       std::cout<<" and "<<deprocess_class_name;
       #endif //!DO_GPU_SEQ_QUEUE
       #endif //!DO_GPU_NO_QUEUE
#endif
    std::cout<<")"<<std::endl;
  }//if
  return 0;
}//main

