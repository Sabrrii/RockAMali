#ifndef _DATA_PROCESSOR_VMVPV_
#define _DATA_PROCESSOR_VMVPV_

#include "CDataProcessor.hpp"

//! complex operation for CPU process (intended for GPU lambda)
/**
 * val+val*val
**/
template<typename Tdata, typename Tproc>
void kernelCPU_vPvMv(CImg<Tdata> &in,CImg<Tproc> &out)
{
  out=in;
  cimg_forX(in,x) out(x)+=in(x)*in(x);
}//kernelCPU_vPvMv
//! complex operation for CPU process (intended for GPU lambda)
/**
 * val+val*val
**/
template<typename Tdata, typename Tproc=Tdata, typename Taccess=unsigned char>
class CDataProcessor_vPvMv : public CDataProcessor<Tdata,Tproc, Taccess>
{
public:
  CDataProcessor_vPvMv(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessor<Tdata,Tproc, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check)
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
      CImg<Tproc> imgt(img);
      kernelCPU(img,imgt);
      return (this->image==imgt);
    }//do_check
    return true;
  }//check_data

  //! compution kernel for an iteration
  virtual void kernelCPU(CImg<Tdata> &in,CImg<Tproc> &out)
  {
    kernelCPU_vPvMv(in,out);
  };//kernelCPU

  //! compution kernel for an iteration
  virtual void kernel(CImg<Tdata> &in,CImg<Tproc> &out)
  {
    kernelCPU(in,out);
  };//kernel

};//CDataProcessor_vPvMv

#endif //_DATA_PROCESSOR_VMVPV_

