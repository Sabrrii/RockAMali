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
template<typename Tdata, typename Taccess=unsigned char>
class CDataProcessorGPU : public CDataProcessor<Tdata, Taccess>
{
public:
  compute::context ctx;
  compute::command_queue queue;

  // create vectors on the device
  compute::vector<Tdata> device_vector1;
  compute::vector<Tdata> device_vector3;

  CDataProcessorGPU(std::vector<omp_lock_t*> &lock
  , compute::device device, int VECTOR_SIZE
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessor<Tdata, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check)
  , ctx(device), queue(ctx, device)
  , device_vector1(VECTOR_SIZE, ctx), device_vector3(VECTOR_SIZE, ctx)
  {
    this->debug=true;
    this->class_name="CDataProcessorGPU";
    this->image.assign(VECTOR_SIZE);
    this->check_locks(lock);
  }//constructor

  //! compution kernel for an iteration (compution=copy, here)
  virtual void kernelGPU(compute::vector<Tdata> &in,compute::vector<Tdata> &out)
  {
    //compute with lambda
    using compute::lambda::_1;
    compute::transform(in.begin(), in.end(), out.begin(),
      _1 , queue);
  };//kernelGPU

  //! compution kernel for an iteration
  virtual void kernel(CImg<Tdata> &in,CImg<Tdata> &out)
  {
    //copy CPU to GPU
    compute::copy(in.begin(), in.end(), device_vector1.begin(), queue);
    //compute
    kernelGPU(device_vector1,device_vector3);
    //copy GPU to CPU
    compute::copy(device_vector3.begin(), device_vector3.end(), out.begin(), queue);
    //wait for completion
    queue.finish();
  };//kernel

};//CDataProcessorGPU

//! complex operation with lambda for GPU process
/**
 * 
**/
template<typename Tdata, typename Taccess=unsigned char>
class CDataProcessorGPU_lambda : public CDataProcessorGPU<Tdata, Taccess>
{
public:
  CDataProcessorGPU_lambda(std::vector<omp_lock_t*> &lock
  , compute::device device, int VECTOR_SIZE
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessorGPU<Tdata, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check)
  {
    this->debug=true;
    this->class_name="CDataProcessorGPU_lambda";
    this->check_locks(lock);
  }//constructor

  virtual bool check_data(CImg<Tdata> &img, int i)
  {
//std::cout<<__FILE__<<"::"<<__func__<<"(...)"<<std::endl;
    if(this->do_check)
    {
      CImg<Tdata> imgt(img);
      imgt=img+img;//*img;
      return (this->image==imgt);
    }//do_check
    return true;
  }//check_data

  //! compution kernel for an iteration (compution=copy, here)
  virtual void kernelGPU(compute::vector<Tdata> &in,compute::vector<Tdata> &out)
  {
    //compute with lambda
    using compute::lambda::_1;
    compute::transform(in.begin(), in.end(), out.begin(),
      _1+_1/**_1*/ , this->queue);
  };//kernelGPU

};//CDataProcessorGPU_lambda

#endif //_DATA_PROCESSOR_GPU_

