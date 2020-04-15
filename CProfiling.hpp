#ifndef _SEQUENTIAL_PROFILING_
#define _SEQUENTIAL_PROFILING_

//! profiling sequential process
/**
 * 
**/
template<typename Tnetcdf>
class CProfilingSequential
{
public:
  std::string class_name;
  bool debug;

  //times
  std::chrono::high_resolution_clock::time_point tp1;

#ifdef DO_NETCDF
  std::string file_name;
  CImgListNetCDF<Tnetcdf> nc;
  CImgList<Tnetcdf> nc_img;//temporary image for type conversion
  //dimension names
  std::vector<std::string> dim_names;
  std::string dim_time;
  //variable names (and its unit)
  std::vector<std::string> var_names;
  std::vector<std::string> unit_names;
  //elapsed time var.
  NcVar *pNCvarETime;
  NcVar *pNCvarETimePIt;
#endif //DO_NETCDF

  CProfilingSequential(std::string nc_file_name
  , std::string &process_class_name, std::string &store_class_name
  , int width
  , std::string program_version
  )
  {
    debug=false;
    class_name="CProfilingSequential";
#ifdef DO_NETCDF
    file_name=nc_file_name;
    //CImgList
    nc_img.assign(2, 1,1,1,1, -99);
    //NetCDF
    std::cout << "CImgListNetCDF::saveNetCDFFile(" << file_name << ",...) return " << nc.saveNetCDFFile((char*)file_name.c_str()) << std::endl;
    dim_time="dimF";
    dim_names.push_back("dim1");
    std::cout << "CImgListNetCDF::addNetCDFDims(" << file_name << ",...) return " << nc.addNetCDFDims(nc_img,dim_names,dim_time) << std::endl<<std::flush;
    //variable names (and its unit)
    var_names.push_back("iteration");
    var_names.push_back("storage");
    unit_names.push_back("us");
    unit_names.push_back("us");
    std::cout << "CImgListNetCDF::addNetCDFVar(" << file_name << ",...) return " << nc.addNetCDFVar(nc_img,var_names,unit_names) << std::endl<<std::flush;
    if (!(nc.pNCvars[0]->add_att("kernel",process_class_name.c_str()))) std::cerr<<"error: for profiling in NetCDF, while adding kernel name attribute (NC_ERROR)."<<std::endl;
    if (!(nc.pNCvars[0]->add_att("frame_size",width))) std::cerr<<"error: for profiling in NetCDF, while adding storage size name attribute (NC_ERROR)."<<std::endl;
    if (!(nc.pNCvars[1]->add_att("storage",store_class_name.c_str()))) std::cerr<<"error: for profiling in NetCDF, while adding storage name attribute (NC_ERROR)."<<std::endl;
    if (!(nc.pNCvars[1]->add_att("frame_size",width))) std::cerr<<"error: for profiling in NetCDF, while adding storage size name attribute (NC_ERROR)."<<std::endl;
    //elapsed time var.
    pNCvarETime=nc.pNCFile->add_var("process_sequential_elapsed_time",ncInt);//us
    pNCvarETime->add_att("units","us");
    pNCvarETimePIt=nc.pNCFile->add_var("process_sequential_elapsed_time_per_iteration",ncInt);//us
    pNCvarETimePIt->add_att("units","us");
    //add global attributes
    ///versions
    nc.pNCFile->add_att("process_sequential",program_version.c_str());
    nc.pNCFile->add_att("CImg_NetCDF",CIMG_NETCDF_VERSION);
    nc.pNCFile->add_att("CParameterNetCDF",CDL_PARAMETER_VERSION);
    nc.pNCFile->add_att("NcTypeInfo",NETCDF_TYPE_INFO_VERSION);
#ifdef DO_GPU
    nc.pNCFile->add_att("ClTypeInfo",CL_IMAGE_DATA_TYPE_INFO_VERSION);
#endif //DO_GPU
#endif //DO_NETCDF
    tp1 = std::chrono::high_resolution_clock::now();
  }//constructor

  virtual void set_process_ET(Tnetcdf elapsed_time)
  {
    nc_img(0)(0)=elapsed_time;
  }//set_process_elapsed_time
  virtual void set_store_ET(Tnetcdf elapsed_time)
  {
    nc_img(1)(0)=elapsed_time;
  }//set_store_elapsed_time

  virtual void put_ETs()
  {
    std::cout << "CImgNetCDF::addNetCDFData(" << file_name << ",...) return " << nc.addNetCDFData(nc_img) << std::endl;
  }//set_store_elapsed_time

};//CProfilingSequential

#endif //_SEQUENTIAL_PROFILING_

