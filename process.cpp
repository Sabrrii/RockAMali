//CoolImage
#include "CImg.h"

//! \todo [medium] [Tproc] needed for GPU

//C++ base
#include <iostream>
#include <string>
#include <vector>

//OpenMP
#include <omp.h>

#define VERSION "v0.5.9k"

//thread lock
#include "CDataGenerator_factory.hpp"
#include "CDataProcessorCPU_factory.hpp"
#ifdef DO_GPU
#include "CDataProcessorGPUfactory.hpp"
#endif //DO_GPU
#include "CDataStore.hpp"

using namespace cimg_library;

#define S 0 //sample

//types
#include "SDataTypes.hpp"

int main(int argc,char **argv)
{
  ///command arguments, i.e. CLI option
  cimg_usage(std::string("generate, process and store data.\n" \
  " It uses different GNU libraries (see --info option)\n\n" \
  " usage: ./process -h\n" \
  "        ./process -s 1024 -n 123 -X true -p 1234 -i 10.10.15.1 -w 1234657\n" \
  "\n version: "+std::string(VERSION) + 
#ifdef DO_NETCDF
  "\n          CImg_NetCDF."+std::string(CIMG_NETCDF_VERSION) + 
  "\n          CParameterNetCDF."+std::string(CDL_PARAMETER_VERSION)+
  "\n          NcTypeInfo."+std::string(NETCDF_TYPE_INFO_VERSION)+
#endif //NetCDF
  "\n "+get_compiled_data_types()+
  "\n compilation date:" \
  ).c_str());//cimg_usage

  const char* imagefilename = cimg_option("-o","sample.cimg",std::string("output file name (e.g." +
#ifdef DO_NETCDF
  std::string(" \"-o data.nc\" or ") +
#endif //NetCDF
  std::string(" \"-o data.cimg -d 3\" gives data_???.cimg)")
  ).c_str());//ouput file name
  const int digit=cimg_option("-d",6,  "number of digit for file names");
  const int width=cimg_option("-s",1024, "size   of udp buffer");
  const int count=cimg_option("-n",256,  "number of frames");
  const int nbuffer=cimg_option("-b",12, "size   of vector buffer (total size is b*s*4 Bytes)");
  const int threadCount=cimg_option("-c",3,"thread count (threads above 2 are processing one)");
//! generator factory
  const std::string generator_type=cimg_option("--generator-factory","count","generator type, e.g. count, random or peak");
  //show type list in generator factory
  std::vector<std::string> generator_type_list;CDataGenerator_factory<Tdata, Taccess>::show_factory_types(generator_type_list);std::cout<<std::endl;
//! CPU processor factory
  const std::string processor_type=cimg_option("--CPU-factory","count","CPU processing type, e.g. count or kernel");
  //show type list in CPU processor factory
  std::vector<std::string> cpu_type_list;CDataProcessorCPU_factory<Tdata,Tproc, Taccess>::show_factory_types(cpu_type_list);std::cout<<std::endl;
#ifdef DO_GPU
//! GPU processor factory
  const bool use_GPU_G=cimg_option("-G",false,NULL);//-G hidden option
        bool use_GPU=cimg_option("--use-GPU",use_GPU_G,"use GPU for compution (or -G option)");use_GPU=use_GPU_G|use_GPU;//same --use-GPU or -G option
  const std::string processing_type=cimg_option("--GPU-factory","program","GPU processing type, e.g. program or function");
  //show type list in GPU processor factory
  std::vector<std::string> gpu_type_list;CDataProcessorGPUfactory<Tdata,Tproc, Taccess>::show_factory_types(gpu_type_list);std::cout<<std::endl;
#endif //DO_GPU
  const bool do_check_C=cimg_option("-C",false,NULL);//-G hidden option
        bool do_check=cimg_option("--do-check",do_check_C,"do data check, e.g. test pass (or -C option)");do_check=do_check_C|do_check;//same --do_check or -C option

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
  CImgList<Tproc> results(nbuffer,width,1,1,1);
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
      //CDataGenerator_Random<Tdata,Taccess> generate(locks);
      CDataGenerator<Tdata, Taccess> *generate=CDataGenerator_factory<Tdata, Taccess>::NewCDataGenerator(generator_type,generator_type_list
      , locks
      );
      std::cout<<"information: generator type is the one in "<<generate->class_name<<" class."<<std::endl<<std::flush;
      generate->run(access,images, count);
      break;
    }//generate
    case 1:
    {//store
      CDataStore<Tproc,Taccess> store(locksR, imagefilename,digit, CDataAccess::STATUS_FILLED);
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
      CDataProcessorGPU<Tdata,Tproc, Taccess> *process=CDataProcessorGPUfactory<Tdata,Tproc, Taccess>::NewCDataProcessorGPU(processing_type,gpu_type_list
      , locks, gpu,width
      , CDataAccess::STATUS_FILLED, CDataAccess::STATUS_FREE  //images
      , CDataAccess::STATUS_FREE,   CDataAccess::STATUS_FILLED//results
      , do_check
      );
      std::cout<<"information: processing type is the one in "<<process->class_name<<" class."<<std::endl<<std::flush;
      process->run(access,images, accessR,results, count, stride,start);
      process->show_checking();
      }//GPU
      else
#endif
      {//CPU
      std::cout<<"information: use CPU for processing (from "<<start<<" by step of "<<stride<<"."<<std::endl<<std::flush;
      CDataProcessor<Tdata,Tproc, Taccess>  *process=CDataProcessorCPU_factory<Tdata,Tproc, Taccess>::NewCDataProcessorCPU(processor_type,cpu_type_list
      , locks
      , CDataAccess::STATUS_FILLED, CDataAccess::STATUS_FREE  //images
      , CDataAccess::STATUS_FREE,   CDataAccess::STATUS_FILLED//results
      , do_check
      );
      std::cout<<"information: processing type is the one in "<<process->class_name<<" class."<<std::endl<<std::flush;
      process->run(access,images, accessR,results, count, stride,start);
      process->show_checking();
      }//CPU
      break;
    }//process
  }//switch(id)
  }//parallel section

  access.print("access (free state)",false);fflush(stderr);
  images.print("CImgList",false);

  accessR.print("accessR (free state)",false);fflush(stderr);
  results.print("CImgListR",false);

        images[0].print("images[0]");
        #if cimg_display!=0   
         if(show) images[0].display_graph("images[0]");
        #endif

  return 0;
}//main

