#ifndef _DATA_PROCESSOR_GPU_
#define _DATA_PROCESSOR_GPU_

//Packages Boost
#include <vector>
#include <iostream>
#include <boost/compute.hpp>
#ifdef DO_GPU_PROFILING
#include <boost/compute/async/future.hpp>
#ifdef DO_NETCDF
#include "CImg_NetCDF.h"
#endif //DO_NETCDF
#endif //DO_GPU_PROFILING

//Package CImg
#include <CImg.h>

namespace compute = boost::compute;

using namespace cimg_library;

using compute::lambda::_1;

#include "CDataProcessor.hpp"

//! base for GPU process 
/**
 * kernel do a simple copy of the data
 * \note presently, this class launch GPU compution on a single data vector and wait for result (this is very slow !)
 * , please use queueing and dequeuing classes to have great performances.
 * This class might be used for debug or test only.
**/
template<typename Tdata, typename Tproc, typename Taccess=unsigned char
#ifdef DO_GPU_PROFILING
#ifdef DO_NETCDF
, typename Tnetcdf=int
#endif //DO_NETCDF
#endif //DO_GPU_PROFILING
>
class CDataProcessorGPU : public CDataProcessor<Tdata,Tproc, Taccess>
{
public:
  compute::context ctx;
  compute::command_queue queue;
#ifdef DO_GPU_PROFILING
  //profiling
  compute::future<void> future;
#ifdef DO_NETCDF
  std::string file_name="profiling_gpu.nc";
  CImgNetCDF<Tnetcdf> nc;
  CImg<Tnetcdf> nc_img;//temporary image for type conversion
  bool is_netcdf_init;
  //dimension names
  std::vector<std::string> dim_names;
  std::string dim_time;
  //variable names (and its unit)
  std::string var_name;
  std::string unit_name;
#endif //DO_NETCDF
#endif //DO_GPU_PROFILING

  // create vectors on the device
  compute::vector<Tdata> device_vector_in;
  compute::vector<Tproc> device_vector_out;


  CDataProcessorGPU(std::vector<omp_lock_t*> &lock
  , compute::device device, int VECTOR_SIZE
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessor<Tdata,Tproc, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check)
  , ctx(device)
  , queue(ctx, device
   #ifdef DO_GPU_PROFILING
    ,compute::command_queue::enable_profiling
   #endif //DO_GPU_PROFILING
    )
  , device_vector_in(VECTOR_SIZE, ctx), device_vector_out(VECTOR_SIZE, ctx)
  {
//! \todo [low] ? need two VECTOR_SIZE: in and out (or single output is done by CPU ?)
//    this->debug=true;
    this->class_name="CDataProcessorGPU";
    this->image.assign(VECTOR_SIZE);
#ifdef DO_NETCDF
    nc_img.assign(1,1,1,1,-99);
std::cout << "CImgNetCDF::saveNetCDFFile(" << file_name << ",...) return " << nc.saveNetCDFFile((char*)file_name.c_str()) << std::endl;
    is_netcdf_init=false;
    dim_time="dimF";
    dim_names.push_back("dimP");
    //variable names (and its unit)
    var_name="kernel_elapsed_time";
    unit_name="us";
std::cout << "CImgNetCDF::addNetCDFDims(" << file_name << ",...) return " << nc.addNetCDFDims(nc_img,dim_names,dim_time) << std::endl;if(this->debug) std::cout<<std::flush;
std::cout << "CImgNetCDF::addNetCDFVar(" << file_name << ",...) return " << nc.addNetCDFVar(nc_img,var_name,unit_name) << std::endl;  if(this->debug) std::cout<<std::flush;
#endif //DO_NETCDF
    this->check_locks(lock);
  }//constructor

  #ifdef DO_GPU_PROFILING
  virtual void kernel_elapsed_time()
  {
    //close elapsed time
    future.wait();
    // get elapsed time from event profiling information
    compute::event evt=future.get_event();
    boost::chrono::microseconds duration=future.get_event().duration<boost::chrono::microseconds>();
    // print elapsed time in microseconds
    std::cout << "[compute] GPU kernel time: " << duration.count() << " us" << std::endl;
   #ifdef DO_NETCDF
    if(!is_netcdf_init)
    {
      //add class name in NetCDF profiling file
      if (!(nc.pNCvar->add_att("kernel",this->class_name.c_str()))) std::cerr<<"error: for profiling in NetCDF, while adding kernel name attribute"<<this->class_name<<" (NC_ERROR)."<<std::endl;
      is_netcdf_init=true;
    }//!is_netcdf_init
    //add data to NetCDF profiling file
    nc_img(0)=duration.count();
if(this->debug) nc_img.print("profiling",false);
std::cout << "CImgNetCDF::addNetCDFData(" << file_name << ",...) return " << nc.addNetCDFData(nc_img) << std::endl;
   #endif //DO_NETCDF
  }//kernel_elapsed_time
  #endif //DO_GPU_PROFILING

  //! compution kernel for an iteration (compution=copy, here)
  virtual void kernelGPU(compute::vector<Tdata> &in,compute::vector<Tproc> &out)
  {
    //compute with lambda
    using compute::lambda::_1;
    compute::transform(in.begin(), in.end(), out.begin(),
      _1 , queue);
  };//kernelGPU

  //! compution kernel for an iteration
  virtual void kernel(CImg<Tdata> &in,CImg<Tproc> &out)
  {
    //copy CPU to GPU
   #ifdef DO_GPU_PROFILING
    this->future=compute::copy_async
   #else
    compute::copy
   #endif //DO_GPU_PROFILING
    (in.begin(), in.end(), device_vector_in.begin(), queue);
    //compute
    kernelGPU(device_vector_in,device_vector_out);
    //copy GPU to CPU
    compute::copy(device_vector_out.begin(), device_vector_out.end(), out.begin(), queue);
    //wait for completion
    queue.finish();
   #ifdef DO_GPU_PROFILING
    kernel_elapsed_time();
   #endif //DO_GPU_PROFILING
  };//kernel

};//CDataProcessorGPU

#endif //_DATA_PROCESSOR_GPU_

