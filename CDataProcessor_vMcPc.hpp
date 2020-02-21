#ifndef _DATA_PROCESSOR_VMCPC_
#define _DATA_PROCESSOR_VMCPC_

#include "CDataProcessor.hpp"

//! complex operation for CPU process (primary intended for GPU lambda)
/**
 * FMA: val*cst+cst, e.g. val * 2.1 + 123.45
**/
template<typename Tdata, typename Tproc>
void kernelCPU_vMcPc(CImg<Tdata> &in,CImg<Tproc> &out)
{
  out=in*2.1 + 123.45;
}//kernelCPU_vMcPc
//! complex operation for CPU process (primary intended for GPU lambda)
/**
 * FMA: val*cst+cst
**/
template<typename Tdata, typename Tproc=Tdata, typename Taccess=unsigned char>
class CDataProcessor_kernel : public CDataProcessor<Tdata,Tproc, Taccess>
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
  : CDataProcessor<Tdata,Tproc, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check)
  {
//    this->debug=true;
    this->class_name="CDataProcessor_kernel_vMcPc";
//std::cout<<__FILE__<<"::"<<__func__<<"(...)"<<std::endl;
//    pKernel4CPU=kernel4CPU;
    this->check_locks(lock);
  }//constructor

  //! compution kernel for an iteration
  virtual void kernelCPU(CImg<Tdata> &in,CImg<Tproc> &out)
  {
//    *pKernelCPU(in,out);
    kernelCPU_vMcPc(in,out);
  };//kernelCPU

  virtual bool check_data(CImg<Tdata> &img, int i)
  {
//std::cout<<__FILE__<<"::"<<__func__<<"/"<<this->class_name<<"(...)"<<std::endl;
    if(this->do_check)
    {
      CImg<Tproc> imgt(img);
      kernelCPU(img,imgt);
      return (this->image==imgt);
    }//do_check
    return true;
  }//check_data

  //! compution kernel for an iteration
  virtual void kernel(CImg<Tdata> &in,CImg<Tproc> &out)
  {
    kernelCPU(in,out);
  };//kernel

};//CDataProcessor_kernel
	
#endif //_DATA_PROCESSOR_VMCPC_

