#ifndef _DATA_PROCESSOR_GPU_QUEUE_
#define _DATA_PROCESSOR_GPU_QUEUE_


//Packages Boost
#include <vector>
#include <iostream>
#include <boost/compute.hpp>
#include <boost/compute/async/future.hpp>

//Package CImg
#include <CImg.h>

namespace compute = boost::compute;

using namespace cimg_library;

using compute::lambda::_1;

#include "CDataProcessorGPU.hpp"

//! base queueing for GPU process (ment for enqueue and dequeue)
/**
 * This class might be used for debug or test only.
**/
template<typename Tdata, typename Taccess=unsigned char>
class CDataProcessorGPUqueue : public CDataProcessorGPU<Tdata, Taccess>
{
public:
  compute::future<void> lwait;
  CDataProcessorGPUqueue(std::vector<omp_lock_t*> &lock
  , compute::device device, int vector_size
  , CImgList<Tdata> &images, compute::future<void> &lwait //out
  , compute::vector<Tdata> *ldevice_vector_in, compute::vector<Tdata> *ldevice_vector_out //out (or in)
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  , bool do_out=true
  )
  : CDataProcessorGPU<Tdata, Taccess>(lock,device,vector_size,wait_status,set_status,wait_statusR,set_statusR,do_check)
  , lwait(lwait)
  {
    this->debug=true;
    this->class_name="CDataProcessorGPUqueue";
    //check data size
    if(images(0).width()!=vector_size) {std::cout<< __FILE__<<"/"<<__func__;printf("(...) code error: bad image size"); exit(99);}
    if(do_out)
    {
      ldevice_vector_in=&(this->device_vector_in);
      ldevice_vector_out=&(this->device_vector_out);
    }//out
    this->check_locks(lock);
  }//constructor

  //! compution kernel for an iteration (compution=copy, here)
  virtual void kernelGPU(compute::vector<Tdata> &in,compute::vector<Tdata> &out
  , compute::command_queue &queue)
  {
    //compute with lambda
    using compute::lambda::_1;
    compute::transform(in.begin(), in.end(), out.begin(),
      _1 , queue);
  };//kernelGPU

  //! compution kernel for an enqueue iteration (using queue)
  virtual void kernel(CImg<Tdata> &in,CImg<Tdata> &out)
  {
//! \note async op.: https://github.com/boostorg/compute/issues/303 (enqueue_nd_range_kernel() are already asynchronous: boost::compute::event e = queue.enqueue_barrier();)
    //copy CPU to GPU
    compute::copy(in.begin(), in.end(), this->device_vector_in.begin(), this->queue);
    //compute
    kernelGPU(this->device_vector_in,this->device_vector_out,this->queue);
    //copy GPU to CPU
    lwait=compute::copy_async(this->device_vector_out.begin(), this->device_vector_out.end(), out.begin(), this->queue);
  };//kernel

  //! one iteration for any enqueue loop
  virtual void iteration_enqueue(CImg<Taccess> &access,CImgList<Tdata> &bimages, CImg<Taccess> &accessR,CImgList<Tdata> &results, int n, int i)
  {
    if(this->debug)
    {
      this->lprint.print("",false);
      printf("4 B%02d #%04d: ",n,i);fflush(stdout);
      access.print("access",false);fflush(stderr);
      this->lprint.unset_lock();
    }

    //! 1. compute from buffer
    //wait lock
    unsigned int c=0;
    this->laccess.wait_for_status(access[n],this->wait_status,this->STATE_ENQUEUEING, c);//filled, processing
    //compution in local
    kernel(bimages[n],this->image);

        //check
        if(this->do_check)
        {
          if(bimages[n]==i) NULL; else {++(this->check_error);std::cout<<"compution error: bad generate class for this test."<<std::endl<<std::flush;}
        }

    //unlock
    this->laccess.set_status(access[n],this->STATE_ENQUEUEING,/*this->set_status*/this->STATUS_QUEUED, this->class_name[5],i,n,c);//processing, processed

  }//iteration_enqueue

  //! one iteration for any dequeue loop
  virtual void iteration_dequeue(CImg<Taccess> &access,CImgList<Tdata> &bimages, CImg<Taccess> &accessR,CImgList<Tdata> &results, int n, int i)
  {
    if(this->debug)
    {
      this->lprint.print("",false);
      printf("4 B%02d #%04d: ",n,i);fflush(stdout);
      access.print("access",false);fflush(stderr);
      this->lprint.unset_lock();
    }

    //! 1. compute from buffer
    //wait lock
    unsigned int c=0;
    this->laccess.wait_for_status(access[n],/*this->wait_status*/this->STATUS_QUEUED,this->STATE_PROCESSING, c);//filled, processing
    //compution in local
    this->lwait.wait();
    //unlock
    this->laccess.set_status(access[n],this->STATE_PROCESSING,this->set_status, this->class_name[5],i,n,c);//processing, processed

    //! 2. copy to buffer
    if(this->debug)
    {
      this->lprint.print("",false);
      printf("4 C%02d #%04d: ",n,i);fflush(stdout);
      accessR.print("accessR",false);fflush(stderr);
      this->lprint.unset_lock();
    }

        //check
        if(this->do_check)
        {
          if(this->image==i) NULL;
          else
          {
            ++(this->check_error);
            std::cout<<"compution error: bad check (i.e. test failed) on iteration #"<<i<<" (value="<<
            this->image(0)
            <<")."<<std::endl<<std::flush;
          }
        }

    //wait lock
    c=0;
    this->laccessR.wait_for_status(accessR[n],this->wait_statusR,this->STATE_PROCESSING, c);//filled, processing
    //copy local to buffer
    results[n]=this->image;
    //unlock
    this->laccessR.set_status(accessR[n],this->STATE_PROCESSING,this->set_statusR, this->class_name[5],i,n,c);//processing, processed

  }//iteration_dequeue

  virtual void iteration(CImg<Taccess> &access,CImgList<Tdata> &bimages, CImg<Taccess> &accessR,CImgList<Tdata> &results, int n, int i)
  {
    std::cout<< __FILE__<<"/"<<__func__<<" 1. enqueue"<<std::endl<<std::flush;
    this->iteration_enqueue(access,bimages, accessR,results, n,i);
    std::cout<< __FILE__<<"/"<<__func__<<" 2. dequeue"<<std::endl<<std::flush;
    this->iteration_dequeue(access,bimages, accessR,results, n,i);
    std::cout<< __FILE__<<"/"<<__func__<<" v. done"<<std::endl<<std::flush;
  }//iteration

};//CDataProcessorGPUqueue

//! enqueueing for GPU process
template<typename Tdata, typename Taccess=unsigned char>
class CDataProcessorGPUenqueue : public CDataProcessorGPUqueue<Tdata, Taccess>
{
public:
  CDataProcessorGPUenqueue(std::vector<omp_lock_t*> &lock
  , compute::device device, int vector_size
  , CImgList<Tdata> &images, compute::future<void> &lwait //out
  , compute::vector<Tdata> *ldevice_vector_in, compute::vector<Tdata> *ldevice_vector_out //out
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessorGPUqueue<Tdata, Taccess>(lock,device,vector_size
    ,images, lwait,ldevice_vector_in,ldevice_vector_out
    ,wait_status,set_status,wait_statusR,set_statusR
    ,do_check
    )
  {
    this->debug=true;
    this->class_name="CDataProcessorGPUenqueue";
    this->check_locks(lock);
  }//constructor

  virtual void iteration(CImg<Taccess> &access,CImgList<Tdata> &bimages, CImg<Taccess> &accessR,CImgList<Tdata> &results, int n, int i)
  {
    this->iteration_enqueue(access,bimages, accessR,results, n,i);
  }//iteration
};//CDataProcessorGPUenqueue

//! dequeueing for GPU process
template<typename Tdata, typename Taccess=unsigned char>
class CDataProcessorGPUdequeue : public CDataProcessorGPUqueue<Tdata, Taccess>
{
public:
  CDataProcessorGPUdequeue(std::vector<omp_lock_t*> &lock
  , compute::device device, int vector_size
  , CImgList<Tdata> &images, compute::future<void> &lwait //in
  , compute::vector<Tdata> *ldevice_vector_in, compute::vector<Tdata> *ldevice_vector_out //in
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessorGPUqueue<Tdata, Taccess>(lock,device,vector_size
    ,images, lwait,ldevice_vector_in,ldevice_vector_out
    ,wait_status,set_status,wait_statusR,set_statusR
    ,do_check
    ,false
    )
  {
    this->debug=true;
    this->class_name="CDataProcessorGPUdequeue";
    //in
    this->lwait=lwait;
    this->device_vector_in=*ldevice_vector_in;
    this->device_vector_out=*ldevice_vector_out;

    this->check_locks(lock);
  }//constructor

  virtual void iteration(CImg<Taccess> &access,CImgList<Tdata> &bimages, CImg<Taccess> &accessR,CImgList<Tdata> &results, int n, int i)
  {
    this->iteration_dequeue(access,bimages, accessR,results, n,i);
  }//iteration
};//CDataProcessorGPUdequeue

#endif //_DATA_PROCESSOR_GPU_QUEUE_

