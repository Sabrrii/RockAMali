#ifndef _DATA_PROCESSOR_GPU_FACTORY_
#define _DATA_PROCESSOR_GPU_FACTORY_

#include "CDataProcessorGPU.hpp"

//! factory for GPU processing
/**
 *
**/
template<typename Tdata, typename Taccess=unsigned char>
class CDataProcessorGPUfactory
{
public:
  static CDataProcessorGPU<Tdata, Taccess> *NewCDataProcessorGPU(const std::string &name
  , std::vector<omp_lock_t*> &lock
  , compute::device device, int VECTOR_SIZE
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  {
    if(name == "copy")
      return new CDataProcessorGPU<Tdata, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check);
    if(name == "program")
      return new CDataProcessorGPU_opencl<Tdata, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check);
    if(name == "lambda")
      return new CDataProcessorGPU_lambda<Tdata, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check);
    if(name == "closure")
      return new CDataProcessorGPU_closure<Tdata, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check);
    if(name == "function")
      return new CDataProcessorGPU_function<Tdata, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check);
    if(name == "function_lambda")
      return new CDataProcessorGPU_function_lambda<Tdata, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check);
    if(name == "function_macro")
      return new CDataProcessorGPU_function_macro<Tdata, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check);

//    if(name == "error")
//      return new CDataProcessorGPU_error;
    std::cerr<<"Module name is unknown, i.e. \""<<name<<"\"."<<std::endl;
    return NULL;
  }//NewCDataProcessorGPU
};//CDataProcessorGPUfactory

#endif //_DATA_PROCESSOR_GPU_FACTORY_

