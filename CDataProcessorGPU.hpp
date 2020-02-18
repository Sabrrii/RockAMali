#ifndef _DATA_PROCESSOR_GPU_
#define _DATA_PROCESSOR_GPU_

//Packages Boost
#include <vector>
#include <iostream>
#include <boost/compute.hpp>

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
template<typename Tdata, typename Tproc, typename Taccess=unsigned char>
class CDataProcessorGPU : public CDataProcessor<Tdata,Tproc, Taccess>
{
public:
  compute::context ctx;
  compute::command_queue queue;

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
  , ctx(device), queue(ctx, device)
  , device_vector_in(VECTOR_SIZE, ctx), device_vector_out(VECTOR_SIZE, ctx)
  {
//! \todo [low] ? need two VECTOR_SIZE: in and out (or single output is done by CPU ?)
    this->debug=true;
    this->class_name="CDataProcessorGPU";
    this->image.assign(VECTOR_SIZE);
    this->check_locks(lock);
  }//constructor

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
    compute::copy(in.begin(), in.end(), device_vector_in.begin(), queue);
    //compute
    kernelGPU(device_vector_in,device_vector_out);
    //copy GPU to CPU
    compute::copy(device_vector_out.begin(), device_vector_out.end(), out.begin(), queue);
    //wait for completion
    queue.finish();
  };//kernel

};//CDataProcessorGPU

#endif //_DATA_PROCESSOR_GPU_

