#ifndef _DATA_STORE_
#define _DATA_STORE_

//CoolImage
#include "CImg.h"
using namespace cimg_library;

//NetCDF
//! \todo [high] NetCDF
//! \todo [medium] add optional compilation with NetCDF, i.e. #ifdef for NetCDF
#include "CImg_NetCDF.h"

//thread lock
#include "CDataBuffer.hpp"

//! store data from a shared circular buffer to files
/**
 * this storage data class implements \c iteration function on the data.
 * Data is shared, so both circular access and lock to it should be provided (see parameters: \c images, \c access, \c lock).
 * \todo [low] \c wait_for_status might be a locking process to ensure fastest unlocking for these storage classes.
**/
template<typename Tdata, typename Taccess=unsigned char, typename Tnetcdf=int>
class CDataStore: public CDataBuffer<Tdata, Taccess>
{
public:
  //! output file name or template
  std::string file_name;
  //NetCDF format
  bool is_netcdf_file;
  CImgNetCDF<Tnetcdf> cimgTest;
  CImg<Tnetcdf> nc_img;//temporary image for type conversion
  bool is_netcdf_init;
  //dimension names
  std::vector<std::string> dim_names;
  std::string dim_time;
  //variable names (and its unit)
  std::string var_name;
  std::string unit_name;
  //CImg format
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

//! \todo [high] . force NetCDF
//! \todo [medium] check extention for NetCDF, e.g. setup is_netcdf_file=true
    is_netcdf_file=true;
std::cout << "CImgNetCDF::saveNetCDFFile(" << file_name << ",...) return " << cimgTest.saveNetCDFFile((char*)file_name.c_str()) << std::endl;
    is_netcdf_init=false;
    dim_time="dimF";
    dim_names.push_back("dimS");
    //variable names (and its unit)
    var_name="signal";
    unit_name="none";

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

//! \todo [high] . NetCDF
    if(!is_netcdf_init)
    {
      nc_img.assign(images[n].width());
std::cout << "CImgNetCDF::addNetCDFDims(" << file_name << ",...) return " << cimgTest.addNetCDFDims(nc_img,dim_names,dim_time) << std::endl;
std::cout << "CImgNetCDF::addNetCDFVar(" << file_name << ",...) return " << cimgTest.addNetCDFVar(nc_img,var_name,unit_name) << std::endl;
      is_netcdf_init=true;
    }//init NetCDF

    //{CImg file
    CImg<char> nfilename(1024);
    if(!is_netcdf_file)
      cimg::number_filename(file_name.c_str(),i,file_name_digit,nfilename);
    //}CImg file

    //wait lock
    unsigned int c=0;
    this->laccess.wait_for_status(access[n],this->wait_status,this->STATE_STORING, c, la_sleep);//processed,storing

    //save data
    if(is_netcdf_file)
    {//as NetCDF
      nc_img=images[n];
      std::cout << "CImgNetCDF::addNetCDFData(" << file_name << ",...) return " << cimgTest.addNetCDFData(nc_img) << std::endl;
    }//NetCDF
    else
     //as image
      images[n].save(nfilename);

    //set filled
    this->laccess.set_status(access[n],this->STATE_STORING,this->set_status, this->class_name[5],i,n,c,la_sleep);//storing,free

    //adjust wait (load average)
    if(c>10) la_sleep*=2;
    if(c<5)  la_sleep/=2;
  }//iteration

};//CDataStore


#endif //_DATA_STORE_

