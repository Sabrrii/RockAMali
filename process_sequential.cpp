//CoolImage
#include "CImg.h"

//C++ base
#include <iostream>
#include <string>
#include <vector>
#ifdef DO_PROFILING
//std::chrono
#include <ctime>
#include <ratio>
#include <chrono>
#ifdef DO_NETCDF
#include "CImg_NetCDF.h"
#endif //DO_NETCDF
#endif //DO_PROFILING

//OpenMP
#include <omp.h>

#define VERSION "v0.7.0"

//thread lock
#include "CDataGenerator_factory.hpp"
#include "CDataProcessorCPU_factory.hpp"
#ifdef DO_GPU
#ifdef DO_GPU_NO_QUEUE
#warning "DO_GPU_NO_QUEUE active (this must be CODE TEST only)"
#include "CDataProcessorGPUfactory.hpp"
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
#include "SDataTypes.hpp"

int main(int argc,char **argv)
{
  ///command arguments, i.e. CLI option
  cimg_usage(std::string("generate, process and store data sequentialy.\n" \
  " It uses different GNU libraries (see --info option)\n\n" \
  " usage: ./process -h\n" \
  "        ./process -s 1024 -n 123 -o result.nc\n" \
  "\n version: "+std::string(VERSION) + 
#ifdef DO_NETCDF
  "\n          CImg_NetCDF."+std::string(CIMG_NETCDF_VERSION) + 
  "\n          CParameterNetCDF."+std::string(CDL_PARAMETER_VERSION)+
  "\n          NcTypeInfo."+std::string(NETCDF_TYPE_INFO_VERSION)+
#endif //NetCDF
  "\n "+get_compiled_data_types()+
  "\n compilation date:" \
  ).c_str());//cimg_usage

  const char* imagefilename = cimg_option("-o","samples/sample.cimg",std::string("output file name (e.g." +
#ifdef DO_NETCDF
  std::string(" \"-o data.nc\" or ") +
#endif //NetCDF
  std::string(" \"-o data.cimg -d 3\" gives data_???.cimg)")
  ).c_str());//ouput file name for raw
  const char* resultfilename = cimg_option("-r","results/sample.cimg",std::string("result file name (e.g." +
#ifdef DO_NETCDF
  std::string(" \"-r result.nc\" or ") +
#endif //NetCDF
  std::string(" \"-r result.cimg -d 3\" gives result_???.cimg)")
  ).c_str());//ouput file name for result
  const int digit=cimg_option("-d",6,  "number of digit for file names");
  const int width=cimg_option("-s",1024, "size   of udp buffer");
  const int count=cimg_option("-n",256,  "number of frames");
  const int nbuffer=1;
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
  //show type list in factory
  std::vector<std::string> type_list;CDataProcessorGPUfactory<Tdata,Tproc, Taccess>::show_factory_types(type_list);std::cout<<std::endl;
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

  //do check
  unsigned int check_error=0;
  //info
  std::string   process_class_name;
  std::string deprocess_class_name;

#ifdef DO_GPU
  //Choosing the target for OpenCL computing
  boost::compute::device gpu = boost::compute::system::default_device();
  CImgList<Tproc> limages(nbuffer,width,1,1,1);
  std::vector<compute::future<void>  > waits(nbuffer);//this may be filled in kernel
  compute::vector<Tproc> *device_vector_in;compute::vector<Tproc> *device_vector_out;//need more in process
  #pragma omp parallel shared(print_lock, access,images, accessR,results, check_error, gpu,limages,waits,device_vector_in,device_vector_out)
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
      CDataGenerator<Tdata, Taccess> *generate=CDataGenerator_factory<Tdata, Taccess>::NewCDataGenerator 
      (generator_type, generator_type_list, locks);
      std::cout<<"information: generator type is the one in "<<generate->class_name<<" class."<<std::endl<<std::flush;
     //process
      CDataProcessor<Tdata,Tproc, Taccess> *process;
      CDataProcessor<Tdata,Tproc, Taccess> *deprocess;
#ifdef DO_GPU
      CImgList<Tdata> limages(nbuffer,width,1,1,1);
      if(use_GPU)
      {//GPU
     #ifdef DO_GPU_NO_QUEUE
      std::cout<<"information: use GPU for processing."<<std::endl<<std::flush;
      ///GPU process from factory
      process=CDataProcessorGPUfactory<Tdata,Tproc, Taccess>::NewCDataProcessorGPU(processing_type,type_list
      , locks, gpu,width
      , CDataAccess::STATUS_FILLED,CDataAccess::STATUS_PROCESSED //images
      , CDataAccess::STATUS_FREE,  CDataAccess::STATUS_FILLED    //results
      , do_check
      );
      std::cout<<"information: processing type is the one of "<<process->class_name<<" class."<<std::endl<<std::flush;
     #else //DO_GPU_NO_QUEUE
     #ifdef  DO_GPU_SEQ_QUEUE
      std::cout<<"information: use GPU for processing (sequential queue)."<<std::endl<<std::flush;
      process=new CDataProcessorGPUqueue<Tdata,Tproc, Taccess>(locks, gpu,width
      , limages, waits[0],device_vector_in,device_vector_out
      , CDataAccess::STATUS_FILLED, CDataAccess::STATUS_FREE  //images
      , CDataAccess::STATUS_FREE,   CDataAccess::STATUS_FILLED//results
      , do_check
      );
     #else //!DO_GPU_SEQ_QUEUE
      std::cout<<"information: use GPU for processing (enqueue and dequeue)."<<std::endl<<std::flush;
      process=new CDataProcessorGPUenqueue<Tdata,Tproc, Taccess>(locks, gpu,width
      , limages, waits[0],device_vector_in,device_vector_out
      , CDataAccess::STATUS_FILLED, CDataAccess::STATUS_FREE  //images
      , CDataAccess::STATUS_FREE,   CDataAccess::STATUS_FILLED//results
      , do_check
      );
      deprocess=new CDataProcessorGPUdequeue<Tdata,Tproc, Taccess>(locks, gpu,width
      , limages, waits[0],device_vector_in,device_vector_out
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
      process=CDataProcessorCPU_factory<Tdata,Tproc, Taccess>::NewCDataProcessorCPU(processor_type,cpu_type_list
      , locks
      , CDataAccess::STATUS_FILLED,CDataAccess::STATUS_PROCESSED //images
      , CDataAccess::STATUS_FREE,  CDataAccess::STATUS_FILLED    //results
      , do_check
      );
      }//CPU
      process_class_name=process->class_name;
     //stores
      CDataStore<Tdata,Taccess> store(locks,    imagefilename,digit, CDataAccess::STATUS_PROCESSED);
      CDataStore<Tproc,Taccess> storeR(locksR, resultfilename,digit, CDataAccess::STATUS_FILLED);
#ifdef DO_PROFILING
#ifdef DO_NETCDF
    std::string file_name="profiling_process.nc";
    CImgListNetCDF<Tnetcdf> nc;
    CImgList<Tnetcdf> nc_img;//temporary image for type conversion
    //dimension names
    std::vector<std::string> dim_names;
    std::string dim_time;
    //variable names (and its unit)
    std::vector<std::string> var_names;
    std::vector<std::string> unit_names;
    nc_img.assign(2, 1,1,1,1, -99);
    std::cout << "CImgListNetCDF::saveNetCDFFile(" << file_name << ",...) return " << nc.saveNetCDFFile((char*)file_name.c_str()) << std::endl;
    dim_time="dimF";
    dim_names.push_back("dim1");
    std::cout << "CImgListNetCDF::addNetCDFDims(" << file_name << ",...) return " << nc.addNetCDFDims(nc_img,dim_names,dim_time) << std::endl<<std::flush;
    //variable names (and its unit)
    var_names.push_back("iteration");
    var_names.push_back("storage");
    unit_names.push_back("us");
    unit_names.push_back("us");
std::cout << "CImgListNetCDF::addNetCDFVar(" << file_name << ",...) return " << nc.addNetCDFVar(nc_img,var_names,unit_names) << std::endl<<std::flush;
    if (!(nc.pNCvars[0]->add_att("kernel",process->class_name.c_str()))) std::cerr<<"error: for profiling in NetCDF, while adding kernel name attribute (NC_ERROR)."<<std::endl;
    if (!(nc.pNCvars[0]->add_att("frame_size",width))) std::cerr<<"error: for profiling in NetCDF, while adding storage size name attribute (NC_ERROR)."<<std::endl;
    if (!(nc.pNCvars[1]->add_att("storage",store.class_name.c_str()))) std::cerr<<"error: for profiling in NetCDF, while adding storage name attribute (NC_ERROR)."<<std::endl;
    if (!(nc.pNCvars[1]->add_att("frame_size",width))) std::cerr<<"error: for profiling in NetCDF, while adding storage size name attribute (NC_ERROR)."<<std::endl;
#endif //DO_NETCDF
#endif //DO_PROFILING

      //run
      for(unsigned int i=0;i<count;++i)
      {
        generate->iteration(access,images,0,i);
        images[0].print(generator_type.c_str());
        #if cimg_display!=0   
         if(show) images[0].display_graph(generator_type.c_str());
        #endif
#ifdef DO_PROFILING
       cimg::tic();
       std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
#endif //DO_PROFILING
        process->iteration(access,images, accessR,results, 0,i);
#ifdef DO_PROFILING
       std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
       cimg::toc();
       std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
       std::cout << "iteration elapsed time=" << time_span.count()*1000 << " ms.";
       nc_img(0)(0)=time_span.count()*1000000;//us
       cimg::tic();
       t1 = std::chrono::high_resolution_clock::now();
#endif //DO_PROFILING
        store.iteration(access,images, 0,i);
        storeR.iteration(accessR,results, 0,i);
#ifdef DO_PROFILING
       t2 = std::chrono::high_resolution_clock::now();
       cimg::toc();
       time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
       std::cout << "storage elapsed time=" << time_span.count()*1000 << " ms.";
       nc_img(1)(0)=time_span.count()*1000000;//us
//        std::cout<<"timing: elapsed for process="<<tp<<" ms, store frame="<<ts<<" ms, store result="<<tr<<" ms.";
       std::cout << "CImgNetCDF::addNetCDFData(" << file_name << ",...) return " << nc.addNetCDFData(nc_img) << std::endl;
#endif //DO_PROFILING
        //check
        if(do_check)
        {
 /*         //if(images[0] ==i)
          if(images[0][0] ==i)
          NULL; else {++check_error;std::cout<<"compution error: bad main generate class for this test."<<std::endl<<std::flush;}
          //if(results[0]==i+i*i)
          if(results[0][0]==i*2+123)
          NULL; else {++check_error;std::cout<<"compution error: bad main check (i.e. test failed) on iteration #"<<i<<" (value="<<results[0](0)<<")."<<std::endl<<std::flush;}*/

         process->show_checking();
        }

        results[0].print(processor_type.c_str());
        #if cimg_display!=0   
         if(show) results[0].display_graph(processor_type.c_str());
        #endif
      }//vector loop

#ifdef DO_GPU_PROFILING
#ifdef DO_NETCDF
//! \bug [GPU_PROFILING and NetCDF] force close of file ?!
      if(use_GPU)
      {
        CDataProcessorGPU<Tdata,Tproc, Taccess>*gpuprocess=(CDataProcessorGPU<Tdata,Tproc, Taccess>*)process;
        gpuprocess->nc.pNCFile->close();
      }//use_GPU
#endif //DO_NETCDF
#endif //DO_GPU_PROFILING

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

