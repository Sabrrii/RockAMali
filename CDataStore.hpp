#ifndef _DATA_STORE_
#define _DATA_STORE_

//CoolImage
#include "CImg.h"
using namespace cimg_library;

//thread lock
#include "CDataBuffer.hpp"

//! store data from a shared circular buffer to files
/**
 * this storage data class implements \c iteration function on the data.
 * Data is shared, so both circular access and lock to it should be provided (see parameters: \c images, \c access, \c lock).
 * \todo [low] \c wait_for_status might be a locking process to ensure fastest unlocking for these storage classes.
**/
template<typename Tdata, typename Taccess=unsigned char>
class CDataStore: public CDataBuffer<Tdata, Taccess>
{
public:
  std::string file_name;
  unsigned int file_name_digit;
  unsigned int la_sleep;
//  unsigned int la_count;

  CDataStore(std::vector<omp_lock_t*> &lock
  , std::string imagefilename, unsigned int digit
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_FREE
  )
  : CDataBuffer<Tdata, Taccess>(lock,wait_status,set_status)
  {
//    this->debug=true;
    this->class_name="CDataStore";
    file_name=imagefilename;
    file_name_digit=digit;
    la_sleep=12;//us
    this->check_locks(lock);
  }//constructor

  //! one iteration for any loop
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
    this->laccess.wait_for_status(access[n],this->wait_status,this->STATE_STORING, c, la_sleep);//processed,storing
    //save image
    CImg<char> nfilename(1024);
    cimg::number_filename(file_name.c_str(),i,file_name_digit,nfilename);
    images[n].save(nfilename);
 
    //set filled
    this->laccess.set_status(access[n],this->STATE_STORING,this->set_status, this->class_name[5],i,n,c,la_sleep);//storing,free

    //adjust wait (load average)
    if(c>10) la_sleep*=2;
    if(c<5)  la_sleep/=2;
  }//iteration

};//CDataStore


#endif //_DATA_STORE_

