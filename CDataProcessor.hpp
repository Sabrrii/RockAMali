#ifndef _DATA_PROCESSOR_
#define _DATA_PROCESSOR_


//Package CImg
#include <CImg.h>

using namespace cimg_library;

#include "CDataBuffer.hpp"

template<typename Tdata, typename Taccess=unsigned char>
class CDataProcessor : public CDataBuffer<Tdata, Taccess>
{
public:
  //Processing buffer
  CImg<Tdata> image;
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

  //! compution kernel for an iteration
  virtual void kernel(CImg<Tdata> &in,CImg<Tdata> &out)
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
  virtual void iteration(CImg<Taccess> &access,CImgList<Tdata> &images, CImg<Taccess> &accessR,CImgList<Tdata> &results, int n, int i)
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

//! complex operation for CPU process (intended for GPU lambda)
/**
 * val+val*val
**/
template<typename Tdata>
void kernelCPU_vPvMv(CImg<Tdata> &in,CImg<Tdata> &out)
{
  out=in;
  cimg_forX(in,x) out(x)+=in(x)*in(x);
}//kernelCPU_vPvMv
//! complex operation for CPU process (intended for GPU lambda)
/**
 * val+val*val
**/
template<typename Tdata, typename Taccess=unsigned char>
class CDataProcessor_vPvMv : public CDataProcessor<Tdata, Taccess>
{
public:
  CDataProcessor_vPvMv(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessor<Tdata, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check)
  {
    this->debug=true;
    this->class_name="CDataProcessor_vPvMv";
std::cout<<__FILE__<<"::"<<__func__<<"(...)"<<std::endl;
    this->check_locks(lock);
  }//constructor

  virtual bool check_data(CImg<Tdata> &img, int i)
  {
//std::cout<<__FILE__<<"::"<<__func__<<"/"<<this->class_name<<"(...)"<<std::endl;
    if(this->do_check)
    {
      CImg<Tdata> imgt(img);
      kernelCPU(img,imgt);
      return (this->image==imgt);
    }//do_check
    return true;
  }//check_data

  //! compution kernel for an iteration
  virtual void kernelCPU(CImg<Tdata> &in,CImg<Tdata> &out)
  {
    kernelCPU_vPvMv(in,out);
  };//kernelCPU

  //! compution kernel for an iteration
  virtual void kernel(CImg<Tdata> &in,CImg<Tdata> &out)
  {
    kernelCPU(in,out);
  };//kernel

};//CDataProcessor_vPvMv

//! complex operation for CPU process (intended for GPU lambda)
/**
 * FMA: val*cst+cst
**/
template<typename Tdata>
void kernelCPU_vMcPc(CImg<Tdata> &in,CImg<Tdata> &out)
{
  out=in*2+123;
}//kernelCPU_vMcPc
//! complex operation for CPU process (intended for GPU lambda)
/**
 * FMA: val*cst+cst
**/
template<typename Tdata, typename Taccess=unsigned char>
class CDataProcessor_kernel : public CDataProcessor<Tdata, Taccess>
{
public:
//  void *pKernel4CPU;
  CDataProcessor_kernel(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
//  , void *kernel4CPU(void)
  , bool do_check=false
  )
  : CDataProcessor<Tdata, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check)
  {
    this->debug=true;
    this->class_name="CDataProcessor_kernel_vMcPc";
std::cout<<__FILE__<<"::"<<__func__<<"(...)"<<std::endl;
//    pKernel4CPU=kernel4CPU;
    this->check_locks(lock);
  }//constructor

  //! compution kernel for an iteration
  virtual void kernelCPU(CImg<Tdata> &in,CImg<Tdata> &out)
  {
//    *pKernelCPU(in,out);
    kernelCPU_vMcPc(in,out);
  };//kernelCPU

  virtual bool check_data(CImg<Tdata> &img, int i)
  {
//std::cout<<__FILE__<<"::"<<__func__<<"/"<<this->class_name<<"(...)"<<std::endl;
    if(this->do_check)
    {
      CImg<Tdata> imgt(img);
      kernelCPU(img,imgt);
      return (this->image==imgt);
    }//do_check
    return true;
  }//check_data

  //! compution kernel for an iteration
  virtual void kernel(CImg<Tdata> &in,CImg<Tdata> &out)
  {
    kernelCPU(in,out);
  };//kernel

};//CDataProcessor_kernel

template<typename Tdata, typename Taccess=unsigned char>
class CDataProcessor_Max_Min : public CDataProcessor_kernel<Tdata, Taccess>
{
public:
  

  CDataProcessor_Max_Min(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessor_kernel<Tdata, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check)
  {
    this->debug=true;
    this->class_name="CDataProcessor_Max_Min";
std::cout<<__FILE__<<"::"<<__func__<<"(...)"<<std::endl;
    this->image.assign(1);//content: E only
    this->check_locks(lock);
  }//constructor

  virtual void Process(CImg<Tdata> &in, int &A, int &B, int &Ai, int &Hi, int &Ti, int &threshold) 
  {
	//find the min and max Amplitude
	B = in.min();
	A = in.max() - B;
	//finding the trigger position
	for (int i=0;in(i)== B; i++)
	{
	  Ti=i;
	}
	// finding the position of the maximum Amplitude		
	for (int i=0; in(i)< A + B; i++)
	{
	   Ai= i+1;
	} 		           
	// find the position of 36.8% amplitude and the time
	threshold = A*0.368 + B;
	Hi=Ai;
	while (in(Hi) > threshold) 
  	{
	   Hi++; 
	}
  }//Process_Data

  virtual void Display(CImg<Tdata> &signal, int A, int B, int Ai, int Hi, int Ti, int threshold) 
  {
    	CImg<Tdata> imageC;
	imageC.assign(signal.width(),1,1,4,0);
	imageC.get_shared_channel(0)+=signal;
	imageC.get_shared_channel(1)+=threshold;
	imageC.get_shared_channel(2)+=A+B;
	imageC.get_shared_channel(3)+=A+B;
	//put x at baseline while amplitude is > threshold 
	cimg_for_inX(imageC,Ai,Hi,i) imageC(i,0,0,2)=B;
	cimg_for_inX(imageC,Ti,Ai,i) imageC(i,0,0,3)=B;
        imageC.display_graph("red = signal, green = threshold, blue = max and 36.8% height positions, yellow = trigger and max");
  }//Process_Data

  //! compution kernel for an iteration
  virtual void kernelCPU_Max_Min(CImg<Tdata> &in,CImg<Tdata> &out)
  {
    int A,B,Ai,Hi,Ti,threshold;
    Process(in, A,B,Ai,Hi,Ti,threshold);
    Display(in, A,B,Ai,Hi,Ti,threshold);
    out(0)=A;
  };//kernelCPU_Max_Min

  //! compution kernel for an iteration
  virtual void kernelCPU(CImg<Tdata> &in,CImg<Tdata> &out)
  {
    kernelCPU_Max_Min(in,out);
  };//kernelCPU


};//CDataProcessor_Max_Min	

#endif //_DATA_PROCESSOR_
