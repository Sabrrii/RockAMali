#ifndef _DATA_GENERATOR_FACTORY_
#define _DATA_GENERATOR_FACTORY_

#include "CDataGenerator.hpp"
#include "CDataGenerator_PAC.hpp"

//! factory for data generator
/**
 *
**/
template<typename Tdata, typename Taccess=unsigned char>
class CDataGenerator_factory
{
public:
  static CDataGenerator<Tdata, Taccess> *NewCDataGenerator(const std::string &name//="list types"
  , std::vector<std::string> &factory_types
  , std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_FILLED
  )
  {
    //reset
    factory_types.clear();
    //if
    ///base:  CDataGenerator.hpp
    factory_types.push_back      ("count")         ;if(name == factory_types.back())
      return new CDataGenerator<Tdata, Taccess>(lock,wait_status,set_status);
#ifdef DO_NETCDF
    //! NetCDF enabling more classes, e.g. classes that load parameters from .CDL (i.e. .nc)
    //noise:  CDataGenerator.hpp
    factory_types.push_back      ("signal_random")     ;if(name == factory_types.back())
      return new CDataGenerator_Random<Tdata, Taccess>(lock,wait_status,set_status);
    ///PAC peak:  CDataGenerator_PAC.hpp
    factory_types.push_back      ("full_random")       ;if(name == factory_types.back())
      return new CDataGenerator_Full_Random<Tdata, Taccess>(lock,wait_status,set_status);
    factory_types.push_back      ("signal_pac")        ;if(name == factory_types.back())
      return new CDataGenerator_Peak<Tdata, Taccess>(lock,wait_status,set_status);
    factory_types.push_back      ("peak_noise")        ;if(name == factory_types.back())
      return new CDataGenerator_Peak_Noise<Tdata, Taccess>(lock,wait_status,set_status);
#endif //NetCDF
//    if(name == "error")
//      return new CDataGenerator_error;
    //listing known types in factory
    if(name=="list types")
      return NULL;
    //unknown
    std::cerr<<"Generator name is unknown, i.e. \""<<name<<"\"."<<std::endl;
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
    CDataGenerator_factory<Tdata, Taccess>::NewCDataGenerator("list types",factory_types /*dummies then default*/,locks);
  }//get_factory_types
  //! show type list in factory
  static void show_factory_types(std::vector<std::string> &factory_types)
  {
    if(factory_types.empty()) get_factory_types(factory_types);
    std::cout<<"information: -factory- generator types are: ";
    unsigned int i;
    for(i=0;i<factory_types.size()-1;++i)
      std::cout<<factory_types[i]<<", ";
    std::cout  <<factory_types[i]<<".";
  }//show_factory_types
};//CDataGenerator_factory

#endif //_DATA_GENERATOR_FACTORY_

