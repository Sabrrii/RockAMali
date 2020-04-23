#ifndef _DATA_PROCESSOR_
#define _DATA_PROCESSOR_


//Package CImg
#include <CImg.h>

using namespace cimg_library;

#include "CDataBuffer.hpp"

#ifdef DO_NETCDF
#include <netcdfcpp.h>
#include "struct_parameter_NetCDF.h"
#endif //DO_NETCDF

template<typename Tdata, typename Tproc, typename Taccess=unsigned char>
class CDataProcessor : public CDataBuffer<Tdata, Taccess>
{
public:
  //Processing buffer
  CImg<Tproc> image;
  //! result access
  CAccessOMPLock laccessR;
  CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR;
  CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR;

  bool do_check;unsigned int check_error;

  CDataProcessor(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataBuffer<Tdata, Taccess>(lock,wait_status,set_status)
  , laccessR(lock[2])
  , wait_statusR(wait_statusR), set_statusR(set_statusR)
  , do_check(do_check)
  {
//    this->debug=true;
    this->class_name="CDataProcessor";

    //this->check_locks(lock);
    if(lock.size()<3)
    {
      printf("code error: locks should have at least 3 locks for %s class.",this->class_name.c_str());
      exit(99);
    }
    check_error=0;
  }//constructor
  //! run for loop
  virtual void run(CImg<Taccess> &access,CImgList<Tdata> &images,  CImg<Taccess> &accessR,CImgList<Tproc> &results, unsigned int count, unsigned int stride=1, unsigned int start=0)
  {
    unsigned int nbuffer=images.size();
    for(unsigned int n=start,i=start;i<count;i+=stride)
    {
      this->iteration(access,images, accessR,results, n,i);
      //circular buffer
//! \bug needed: nbuffer should be a multiple of process thread number
      n+=stride;
      if(n>nbuffer-1) n=start;
     }//vector loop
  }//run

  //! compution kernel for an iteration
  virtual void kernel(CImg<Tdata> &in,CImg<Tproc> &out)
  {
    std::cout<< __FILE__<<"/"<<__func__<<"(images: in="<<in.width()<<", out="<<out.size()<<") copy kernel, other kernels should be implemented in inherited class."<<std::endl<<std::flush;
    out=in;
  };//kernel

  //! check data operation (test only, as very long compution function, and might lock data access for other threads)
  /**
   * check local result data (i.e. \c image member of this class) regarding to processed input data (i.e. \c img e.g. \c images[n]).
   * processing of input data to result one is on CPU and takes lot of time regarding to normal processing: test only !
   * \warning input data should be locked during this function, e.g. data not available for other threads
  **/
  virtual bool check_data(CImg<Tdata> &img, int i)
  {
    if(do_check)
    {
//std::cout<<__FILE__<<"::"<<__func__<<"(...)"<<std::endl;
//      return (image==img);
      return (image==i);
    }//do_check
    return true;
  }//check_data

  //! one iteration for any loop
  virtual void iteration(CImg<Taccess> &access,CImgList<Tdata> &images, CImg<Taccess> &accessR,CImgList<Tproc> &results, int n, int i)
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
    this->laccess.wait_for_status(access[n],this->wait_status,this->STATE_PROCESSING, c);//filled, processing
    //compution in local
    kernel(images[n],image);

    //check
    if(do_check)
    {
//std::cout<<__FILE__<<"::"<<__func__<<"(...) do check data"<<std::endl;
      //check input
      //if(images[n]==i) //slow check, but entire frame
      if(images[n](0)==i) //fast random check
      NULL; else {++check_error;std::cout<<"compution error: bad generate class for this test."<<std::endl<<std::flush;}
      //check output
      if(!check_data(images[n],i)) {++check_error;std::cout<<"compution error: bad check (i.e. test failed) on iteration #"<<i<<" (value="<<image(0)<<")."<<std::endl<<std::flush;}
    }//do_check

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

    //wait lock
    c=0;
    this->laccessR.wait_for_status(accessR[n],this->wait_statusR,this->STATE_PROCESSING, c);//filled, processing
    //copy local to buffer
    results[n]=image;
    //unlock
    this->laccessR.set_status(accessR[n],this->STATE_PROCESSING,this->set_statusR, this->class_name[5],i,n,c);//processing, processed

  }//iteration

  virtual void show_checking()
  {
    if(do_check)
    {
    if(check_error>0) std::cout<<"test: fail ("<<check_error<<" errors)";
    else std::cout<<"test: pass";
    std::cout<<" (for "<<this->class_name<<")."<<std::endl;
    }//do_check
  }//show_checking

};//CDataProcessor

#endif //_DATA_PROCESSOR_

