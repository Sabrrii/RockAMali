#ifndef _DATA_GENERATOR_
#define _DATA_GENERATOR_

//CoolImage
#include "CImg.h"
using namespace cimg_library;

//thread lock
#include "CDataBuffer.hpp"

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

//! generate random data into a shared circular buffer
/**
 * random data except first one that is frame count value
**/
template<typename Tdata, typename Taccess=unsigned char>
class CDataGenerator_Random: public CDataGenerator<Tdata, Taccess>
{

public:

  CDataGenerator_Random(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_FILLED
  )
  : CDataGenerator<Tdata, Taccess>(lock,wait_status,set_status)
  {
//    this->debug=true;
    this->class_name="CDataGenerator_Random";
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

    //fill image with random numbers
//! \todo [high] setup random limits depending on data type, i.e. Tdata
//    if(i>n)//fast generation
      images[n].rand(0,65535);
    //set frame count value as first array value
    images[n](0)=i;

    //set filled
    this->laccess.set_status(access[n],this->STATE_FILLING,this->set_status, this->class_name[5],i,n,c);//filling,filled
  }//iteration

};//CDataGenerator_Random

//! generate Peak data into a shared circular buffer
/**
 * Peak data except first one that is frame count value
**/
template<typename Tdata, typename Taccess=unsigned char>
class CDataGenerator_Peak: public CDataGenerator<Tdata, Taccess>
{

public:

  CDataGenerator_Peak(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_FILLED
  )
  : CDataGenerator<Tdata, Taccess>(lock,wait_status,set_status)
  {
//    this->debug=true;
    this->class_name="CDataGenerator_Peak";
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

    //fill image with Peak
//! \todo [high] fill images with local defined variables
    int baseline=23;
    int startpeak=510;
    int endpeak = 610;
    int peaksize = 150;
    images[n].fill(baseline);
    cimg_for_inX(images[n],startpeak,endpeak,i) images[n](i)=peaksize+baseline;
    //set frame count value as first array value
    images[n](0)=i;

    //set filled
    this->laccess.set_status(access[n],this->STATE_FILLING,this->set_status, this->class_name[5],i,n,c);//filling,filled
  }//iteration

};//CDataGenerator_Peak

#endif //_DATA_GENERATOR_

