#ifndef _DATA_STORE_
#define _DATA_STORE_

//CoolImage
#include "CImg.h"
using namespace cimg_library;

//NetCDF
#ifdef DO_NETCDF
#include "CImg_NetCDF.h"
#include <boost/preprocessor/stringize.hpp>
#endif //DO_NETCDF

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
#ifdef DO_NETCDF
  //NetCDF format
  bool is_netcdf_file;
  CImgNetCDF<Tnetcdf> nc;
  CImg<Tnetcdf> nc_img;//temporary image for type conversion
  bool is_netcdf_init;
  //dimension names
  std::vector<std::string> dim_names;
  std::string dim_time;
  //variable names (and its unit)
  std::string var_name;
  std::string unit_name;
  std::string long_name;
  std::string factory_type,factory_name;
#endif //NetCDF
  //CImg format
  unsigned int file_name_digit;

  unsigned int la_sleep;
//  unsigned int la_count;

  CDataStore(std::vector<omp_lock_t*> &lock
  , std::string imagefilename, unsigned int digit
#ifdef DO_NETCDF
  , std::string info__from_factory_type, std::string &info__from_factory_name
  , std::vector<std::string> &var_unit_long_names
#endif //NetCDF
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_FREE
  )
  : CDataBuffer<Tdata, Taccess>(lock,wait_status,set_status)
  {
//    this->debug=true;
    this->class_name="CDataStore";
    file_name=imagefilename;
    file_name_digit=digit;

#ifdef DO_NETCDF
    factory_type=info__from_factory_type;
    factory_name=info__from_factory_name;
    //file extention
    std::size_t found=file_name.find_last_of(".");
    std::string extention=file_name.substr(found);
std::cout << "extention ="<<extention<< std::endl<< std::flush;
    if(!extention.compare(".nc")) is_netcdf_file=true; else is_netcdf_file=false;
    if(is_netcdf_file)
    {
      is_netcdf_init=false;
      dim_time="dimF";
      dim_names.push_back("dimS");
      //variable names (and its unit)
      if(var_unit_long_names.size()>0)
      {
        if(var_unit_long_names.size()<3){std::cerr<<__FILE__<<"::"<<__func__<<"() error: NetCDF need for "<<this->class_name<<" class, at least 3 names (variable, unit and long name)"<<std::endl; exit(2);}
        int n=0;
        var_name=var_unit_long_names[n++];
        unit_name=var_unit_long_names[n++];
        long_name=var_unit_long_names[n++];
      }
      else
      {
        var_name="signal";
        unit_name="digit";
        long_name="signal";
      }//default
    }
#endif //NetCDF

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

#ifdef DO_NETCDF
    if(is_netcdf_file) if(!is_netcdf_init)
    {///create NetCDF file and its header
      //create file
std::cout << "CImgNetCDF::saveNetCDFFile(" << file_name << ",...) return " << nc.saveNetCDFFile((char*)file_name.c_str()) << std::endl;
      nc.pNCFile->add_att("architecture",BOOST_PP_STRINGIZE(ARCH));
    //header
      //factory name as global attribute
      nc.pNCFile->add_att(factory_type.c_str(),factory_name.c_str());
      //use width
      nc_img.assign(images[n].width());
std::cout << "CImgNetCDF::addNetCDFDims(" << file_name << ",...) return " << nc.addNetCDFDims(nc_img,dim_names,dim_time) << std::endl<<std::flush;
std::cout << "CImgNetCDF::addNetCDFVar(" << file_name << ",...) return " << nc.addNetCDFVar(nc_img,var_name,unit_name) << std::endl<<std::flush;
      nc.pNCvar->add_att("long_name",long_name.c_str());
      //factory name as var. attribute
      nc.pNCvar->add_att(factory_type.c_str(),factory_name.c_str());
      is_netcdf_init=true;
    }//init NetCDF
#endif //NetCDF

    //{CImg file
    CImg<char> nfilename(1024);
#ifdef DO_NETCDF
    if(!is_netcdf_file)
#endif //NetCDF
      cimg::number_filename(file_name.c_str(),i,file_name_digit,nfilename);
    //}CImg file

    //wait lock
    unsigned int c=0;
    this->laccess.wait_for_status(access[n],this->wait_status,this->STATE_STORING, c, la_sleep);//processed,storing

    //save data
#ifdef DO_NETCDF
    if(is_netcdf_file)
    {//as NetCDF
      nc_img=images[n];
std::cout << "CImgNetCDF::addNetCDFData(" << file_name << ",...) return " << nc.addNetCDFData(nc_img) << std::endl;
    }//NetCDF
    else
#endif //NetCDF
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

