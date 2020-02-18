#ifndef _DATA_GENERATOR_
#define _DATA_GENERATOR_

//CoolImage
#include "CImg.h"
using namespace cimg_library;

//thread lock
#include "CDataBuffer.hpp"

//NetCDF
#ifdef DO_NETCDF
#include <netcdfcpp.h>
#include "struct_parameter_NetCDF.h"
#endif //NetCDF

//! generate data into a shared circular buffer
/**
 * this generation data class implements \c iteration function on the data.
 * Data is shared, so both circular access and lock to it should be provided (see parameters: \c images, \c access, \c lock).
**/
template<typename Tdata, typename Taccess=unsigned char>
class CDataGenerator: public CDataBuffer<Tdata, Taccess>
{

public:

  CDataGenerator(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_FILLED
  )
  : CDataBuffer<Tdata, Taccess>(lock,wait_status,set_status)
  {
//    this->debug=true;
    this->class_name="CDataGenerator";
    this->check_locks(lock);
  }//constructor

  //! one iteration for any loop
  /**
   * entirely filled with the frame count value
  **/
  virtual void iteration(CImg<Taccess> &access,CImgList<Tdata> &images, int n, int i)
  {
    if(this->debug)
    {
      this->lprint.print("",false);
      printf("4 B%02d #%04d: ",n,i);fflush(stdout);
      access.print("access",false);fflush(stderr);
      this->lprint.unset_lock();
    }
    //wait lock
    unsigned int c=0;
    this->laccess.wait_for_status(access[n],this->wait_status,this->STATE_FILLING, c);//free,filling

    //fill image
    images[n].fill(i);

    //set filled
    this->laccess.set_status(access[n],this->STATE_FILLING,this->set_status, this->class_name[5],i,n,c);//filling,filled
  }//iteration

};//CDataGenerator

#ifdef DO_NETCDF

//! generate random data into a shared circular buffer
/**
 * random data except first one that is frame count value
 *
 * parameters NetCDF CDL :
 * rand_min : minimum value of random
 * rand_max : maximum value of random
**/
template<typename Tdata, typename Taccess=unsigned char>
class CDataGenerator_Random: public CDataGenerator<Tdata, Taccess>
{
public:
  Tdata rand_min,rand_max;
  
int Read_Paramaters (Tdata &min_limit, Tdata &max_limit)
{
  ///file name
  std::string fi="parameters.nc";//=cimg_option("-p","parameters.nc","comment");
  int rnd_min,rnd_max;
  ///parameter class
  CParameterNetCDF fp;
  //open file
  int error=fp.loadFile((char *)fi.c_str());
  if(error){std::cerr<<"loadFile return "<< error <<std::endl;return error;}

  float process; 
  std::string process_name="graph";
  //load process variable
  error=fp.loadVar(process,&process_name);
  if(error){std::cerr<<"loadVar return "<< error <<std::endl;return error;}
  std::cout<<process_name<<"="<<process<<std::endl;
   ///rand_min
  std::string attribute_name="rand_min";
  if (error = fp.loadAttribute(attribute_name,rnd_min)!=0){
    std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
    return error;
  }
  std::cout<<"  "<<attribute_name<<"="<<rnd_min<<std::endl;
  ///rand_max
  attribute_name="rand_max";
  if (error = fp.loadAttribute(attribute_name,rnd_max)!=0){
    std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
    return error;
  }
  std::cout<<"  "<<attribute_name<<"="<<rnd_max<<std::endl;
  min_limit=rnd_min; // convert into int
  max_limit=rnd_max; // convert into int
} //Read_Paramaters

  CDataGenerator_Random(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_FILLED
  )
  : CDataGenerator<Tdata, Taccess>(lock,wait_status,set_status)
  {
//    this->debug=true;
    this->class_name="CDataGenerator_Random";
    this->check_locks(lock);
    Read_Paramaters(rand_min, rand_max);
    //! \todo [lowest] setup limits from CDL (e.g. CDL aggregation with ncgen)
 }//constructor

  //! one iteration for any loop
  /**
   * entirely filled with the frame count value
  **/
  virtual void iteration(CImg<Taccess> &access,CImgList<Tdata> &images, int n, int i)
  {
    if(this->debug)
    {
      this->lprint.print("",false);
      printf("4 B%02d #%04d: ",n,i);fflush(stdout);
      access.print("access",false);fflush(stderr);
      this->lprint.unset_lock();
    }
    //wait lock
    unsigned int c=0;
    this->laccess.wait_for_status(access[n],this->wait_status,this->STATE_FILLING, c);//free,filling

    //fill image with random numbers
//    if(i>n)//fast generation
      images[n].rand(rand_min,rand_max);
    //set frame count value as first array value
    images[n](0)=i;

    //set filled
    this->laccess.set_status(access[n],this->STATE_FILLING,this->set_status, this->class_name[5],i,n,c);//filling,filled
  }//iteration

};//CDataGenerator_Random

#endif //NetCDF

#endif //_DATA_GENERATOR_

