#ifndef _DATA_PROCESSOR_CPU_FACTORY_
#define _DATA_PROCESSOR_CPU_FACTORY_

#include "CDataProcessor.hpp"
#include "CDataProcessor_vMvPv.hpp"
#include "CDataProcessor_vMcPc.hpp"
#include "CDataProcessor_energy.hpp"
//! \todo [low] add morpho one "CDataProcessor_morphomath.hpp"

//! factory for data processing on CPU
/**
 *
**/
template<typename Tdata, typename Tproc, typename Taccess=unsigned char>
class CDataProcessorCPU_factory
{
public:
  static CDataProcessor<Tdata,Tproc, Taccess> *NewCDataProcessorCPU(const std::string &name//="list types"
  , std::vector<std::string> &factory_types
  , std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  {
    //reset
    factory_types.clear();
    //if
    ///base:  CDataProcessorGPU.hpp
    factory_types.push_back   ("count")               ;if(name == factory_types.back())
      return new CDataProcessor<Tdata,Tproc, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check);
    ///vMvPv: CDataProcessorGPU_vMvPv.hpp
    factory_types.push_back   ("vPvMv")               ;if(name == factory_types.back())
      return new CDataProcessor_vPvMv<Tdata,Tproc, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check);
    ///vMcPc: CDataProcessorGPU_vMcPc.hpp
    factory_types.push_back   ("kernel")              ;if(name == factory_types.back())
      return new CDataProcessor_kernel<Tdata,Tproc, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check);
    ///energy: CDataProcessorGPU_energy.hpp
    factory_types.push_back   ("pac")                ;if(name == factory_types.back())
      return new CDataProcessor_Max_Min<Tdata,Tproc, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check);
#ifdef DO_NETCDF
    factory_types.push_back   ("filter")             ;if(name == factory_types.back())
      return new CDataProcessor_Trapeze<Tdata,Tproc, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check);
#endif //NETCDF

//    if(name == "error")
//      return new CDataProcessor_error;
    //listing known types in factory
    if(name=="list types")
      return NULL;
    //unknown
    std::cerr<<"Processor (CPU) name is unknown, i.e. \""<<name<<"\"."<<std::endl;
    return NULL;
  }//NewCDataGenerator

  //! get type list in factory
  static void get_factory_types(std::vector<std::string> &factory_types)
  {
    //dummy vars
    std::vector<omp_lock_t*> locks;
    ///dummy OpenMP locks
    omp_lock_t print_lock;omp_init_lock(&print_lock);
    locks.push_back(&print_lock);
    CDataProcessorCPU_factory<Tdata, Taccess>::NewCDataProcessorCPU("list types",factory_types /*dummies then default*/,locks);
  }//get_factory_types
  //! show type list in factory
  static void show_factory_types(std::vector<std::string> &factory_types)
  {
    if(factory_types.empty()) get_factory_types(factory_types);
    std::cout<<"information: -factory- CPU processor types are: ";
    unsigned int i;
    for(i=0;i<factory_types.size()-1;++i)
      std::cout<<factory_types[i]<<", ";
    std::cout  <<factory_types[i]<<".";
  }//show_factory_types
};//CDataProcessorCPU_factory

#endif //_DATA_PROCESSOR_CPU_FACTORY_

