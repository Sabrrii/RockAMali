#ifndef _SET_DATA_TYPES_
#define _SET_DATA_TYPES_

typedef unsigned char Taccess;
typedef unsigned int  Tdata;
typedef float         Tproc;
//typedef unsigned int  Tproc;
//typedef long int  Tproc; //bad for CImg<T>::pixel_type() for openCL kernel (as int64 return)

#ifdef DO_PROFILING
#ifdef DO_NETCDF
typedef int           Tnetcdf;
#endif //DO_NETCDF
#endif //DO_PROFILING

//! data types as a description string
std::string get_compiled_data_types()
{
  std::string str="compiled types <Tdata,Tproc, Taccess(,Tnetcdf)>=<";
  str+=CImg<Tdata>::pixel_type();
  str+=",";
  str+=CImg<Tproc>::pixel_type();
  str+=", ";
  str+=CImg<Taccess>::pixel_type();
#ifdef DO_PROFILING
#ifdef DO_NETCDF
  str+="(, ";
  str+=CImg<Tnetcdf>::pixel_type();
  str+=")";
#endif //DO_NETCDF
#endif //DO_PROFILING
  str+=">";
  return str;
}

#endif //_SET_DATA_TYPES_

