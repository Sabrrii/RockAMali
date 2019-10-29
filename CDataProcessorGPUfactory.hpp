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
  , std::vector<std::string> &factory_types=NULL
  )
  {
    //reset
    factory_types.clear();
    //if
    factory_types.push_back      ("copy")         ;if(name == factory_types.back())
      return new CDataProcessorGPU<Tdata, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check);
    factory_types.push_back      ("program")             ;if(name == factory_types.back())
      return new CDataProcessorGPU_opencl<Tdata, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check);
    factory_types.push_back      ("lambda")              ;if(name == factory_types.back())
      return new CDataProcessorGPU_lambda<Tdata, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check);
    factory_types.push_back      ("closure")              ;if(name == factory_types.back())
      return new CDataProcessorGPU_closure<Tdata, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check);
    factory_types.push_back      ("function")              ;if(name == factory_types.back())
      return new CDataProcessorGPU_function<Tdata, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check);
    factory_types.push_back      ("function_lambda")              ;if(name == factory_types.back())
      return new CDataProcessorGPU_function_lambda<Tdata, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check);
    factory_types.push_back      ("function_macro")              ;if(name == factory_types.back())
      return new CDataProcessorGPU_function_macro<Tdata, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check);

//    if(name == "error")
//      return new CDataProcessorGPU_error;
    //listing known types in factory
    if(name=="list types")
      return NULL;
    //unknown
    std::cerr<<"Module name is unknown, i.e. \""<<name<<"\"."<<std::endl;
    return NULL;
  }//NewCDataProcessorGPU
  static void show_factory_types(std::vector<std::string> &factory_types)
  {
    for(int i=0;i<factory_types.size();++i)
      std::cout<<factory_types[i]<<", ";
  }
};//CDataProcessorGPUfactory

#endif //_DATA_PROCESSOR_GPU_FACTORY_

