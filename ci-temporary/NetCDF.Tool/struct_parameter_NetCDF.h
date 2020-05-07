#ifndef STRUCT_PARAMETER_NETCDF
#define STRUCT_PARAMETER_NETCDF

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <netcdfcpp.h>

#define CDL_PARAMETER_VERSION "v0.5.1"

#include "../NetCDF.Tool/NetCDFinfo.h"

//! 
/**
 * 
 *
 * \code
 *   
 * \endcode
 *
 * \see 
 *
**/
struct CParameterNetCDF
{
//variable
  public:
  //! 
  /**
   * 
   *
   * \code
   *   NcError err(NcError::verbose_nonfatal);
   * \endcode
   *
   * \see 
   *
  **/
  NcError *pNCError;
  //! 
  /**
   * 
   *
   * \code
   *   string f="2D3C.nc";
   *   NcFile dataFile(f.c_str(),NcFile::ReadOnly);//Read
   *   NcFile dataFile(f.c_str(),NcFile::Replace);//Write
   * \endcode
   *
   * \see 
   *
  **/
  NcFile *pNCFile;
  //! NetCDF variable
  /**
   * NetCDF variable
   *
   * \code
   *   
   * \endcode
   *
  **/
  NcVar *var;
//methods
  //! 
  /**
   * 
   *
   * \param  : 
   *
   * \code
   *   
   * \endcode
   *
  **/
  CParameterNetCDF(void)
  {
#if cimg_debug>10
  std::cerr << "CParameterNetCDF::CParameterNetCDF()" << std::endl;
#endif
    pNCError=new NcError(NcError::verbose_nonfatal);
    pNCFile=NULL;
    var=NULL;
  }
  ~CParameterNetCDF(void)
  {
#if cimg_debug>10
  std::cerr << "CParameterNetCDF::~CParameterNetCDF()" << std::endl;
#endif
//    delete pNCFile;
#if cimg_debug>10
  std::cerr << "CParameterNetCDF::~CParameterNetCDF()/pNCFile NOT deleted" << std::endl;
#endif
    delete pNCError;
#if cimg_debug>10
  std::cerr << "CParameterNetCDF::~CParameterNetCDF()/pNCError deleted" << std::endl;
#endif
  }
  int loadFile(char* fileName)
  {
#if cimg_debug>10
  std::cerr << "CParameterNetCDF::loadFile(\"" << fileName << "\")" << std::endl;
#endif
    pNCFile=new NcFile(fileName,NcFile::ReadOnly);
    if(!pNCFile->is_valid()) std::cerr << "CParameterNetCDF::loadFile  Error: \"" << fileName << "\" is not a NetCDF file or doesn't exists." << std::endl;
    return  ((pNCFile->is_valid())?0:1);
  }
  //! 
  /**
   * 
   *
   * \param  : 
   *
   * \code
   *   
   * \endcode
   *
  **/
  int saveFile(char *fileName)
  {
#if cimg_debug>10
  std::cerr << "CParameterNetCDF::saveFile(\"" << fileName << "\")" << std::endl;
#endif
    pNCFile=new NcFile(fileName,NcFile::Replace);
    return pNCFile->is_valid();
  }
  //! 
  /**
   * 
   *
   * \param  : 
   *
   * \code
   *   
   * \endcode
   *
  **/
  int setFile(NcFile *pNetCDFFile)
  {
#if cimg_debug>10
  std::cerr << "CParameterNetCDF::setFile(" << pNetCDFFile << ")" << std::endl;
#endif
    pNCFile=pNetCDFFile;
    return pNCFile->is_valid();
  }
  //! 
  /**
   * 
   *
   * \param  : 
   *
   * \code
   *   
   * \endcode
   *
  **/
  NcFile *getFile(void)
  {
#if cimg_debug>10
  std::cerr << "CParameterNetCDF::getFile()" << std::endl;
#endif
    return pNCFile;
  }
  //! 
  /**
   * 
   *
   * \param  : 
   *
   * \code
   *   
   * \endcode
   *
  **/
  template<typename T> int loadFile(char *fileName,T &value,std::string *var_name=NULL)
  {
#if cimg_debug>10
  std::cerr << "CParameterNetCDF::loadFile(\"" << fileName << "\",\"" << ",<" << (NcTypeInfo<T>::ncStr()) << ">,\"" << ((var_name==NULL)?"default":(*var_name)) << "\")" << std::endl;
#endif
    //load dims
    if(!loadFile(fileName)) return NC_ERROR;
    return loadVar(value,var_name);
  }
  //! set default variable names
  /**
   * generate default variable names
   *
   * \arg var_names structure to fill with default variable names
   * \arg nbVar when saving: don't put any argument or set it to zero, on the contrary when loading: set it to number of variables to load
   *
   * \code
   *   std::vector<std::string> var_names(nbVar+1);
   *   std::vector<std::string> *pVar_names=setDefaultVarNames(var_names,nbVar);
   * \endcode
   *
   * \see setDefaultUnitName
   *
  **/
  std::string *setDefaultVarName(std::string &var_name)
  {//var names
    var_name="var";
    return &var_name;
  }
  //! set default unit names
  /**
   * generate default unit names
   *
   * \arg unit_names structure to fill with default unit names
   *
   * \code
   *   
   * \endcode
   *
   * \see setDefaultVarName
   *
  **/
  std::string *setDefaultUnitName(std::string &unit_name)
  {//var units
    unit_name="none";
    return &unit_name;
  }
  //! load one variable from the NetCDF file
  /**
   * load a single variable of any type from the allready opened NetCDF file
   *
   * \code
   *   //few declarations
   *   //file name
   *   string fi="parameters.nc";
   *   //parameters
   *   int var_int;string int_name="var_int";
   *   float var_float;string float_name="var_float";
   *   //create the NetCDF parameter class
   *   CAnySingleParameterNetCDF anyPrm;
   *   //call and return test function
   *   //load an \c int
   *   anyPrm.loadFile((char*)fi.c_str(),var_int,&int_name);
   *   cout << "return " << var_int << endl;
   *   //load a \c float
   *   anyPrm.loadVar(var_float,&float_name);
   *   cout << "return " << var_float << endl;
   * \endcode
   *
  **/
  int loadVar(std::string &value,std::string *var_name/*=NULL*/)
  {
#if cimg_debug>10
  std::cerr << "CParameterNetCDF::loadVar(<string>,\"" << ((var_name==NULL)?"default":(*var_name)) << "\")" << std::endl;
#endif
//    char *ptr;
    if (!(var=pNCFile->get_var((*var_name).c_str()))) return NC_ERROR;
    value=var->as_string(0);
    return 0;
  }
  template<typename T> int loadVar(T &value,std::string *pVar_name=NULL)
  {
#if cimg_debug>0
  if(NcTypeStr(NcTypeInfo<T>::ncId())==NULL) {std::cerr << "Error in CParameterNetCDF::loadVar(): this NcType is not implemented yet ! see: NcTypeStr(NcTypeInfo<T>::ncId())" << std::endl;return(NC_ERROR);}
#endif
#if cimg_debug>10
  std::cerr << "CParameterNetCDF::loadVar(<" << (NcTypeInfo<T>::ncStr()) << ">,\"" << ((pVar_name==NULL)?"default":(*pVar_name)) << "\")" << std::endl;
#endif
    ///set default var names in needed
    std::string var_name;
    if(pVar_name==NULL) pVar_name=setDefaultVarName(var_name);
    ///get var
    if (!(var=pNCFile->get_var((*pVar_name).c_str()))) return NC_ERROR;
#if cimg_debug>2
    std::cerr << "CParameterNetCDF: check" << (NcTypeInfo<T>::ncId()) << "==" << (var->type()) << std::endl;
#endif
    if( (NcTypeInfo<T>::ncId())==(var->type()) )//check if same type
    {
#if cimg_debug>10
      std::cerr << "CParameterNetCDF: get" << std::endl;
#endif
      if (!(var->get(&value))) return NC_ERROR;
    }
    else//convert to same type
    {
#if cimg_debug>10
      std::cerr << "Information/CParameterNetCDF: convert <" << NcTypeStr(var->type()) << ">" << var_name << " to <" << (NcTypeInfo<T>::ncStr()) << ">" << std::endl;
//! \todo Warning: convert \c NcType to \c T type (actually only \c as_int is implemented)
std::cerr << "Warning/CParameterNetCDF: only read T=int as only 'as_int()' is implemented !!" << std::endl;
#endif
      if (!(value=(var->as_int(0)))) return NC_ERROR;//#####
    }
    return 0;
  }
  //! load array variable from the NetCDF file
  /**
   * load an array variable of any type from the allready opened NetCDF file
   *
   * \code
   *   //few declarations
   *   //file name
   *   std::string fi="parameters.nc";
   *   //parameter
   *   //\c float array
   *   std::vector<float> var_float_array;std::string float_array_name="var_float_array";
   *   //\c string array
   *   std::vector<std::string> var_string_array;std::string string_array_name="var_string_array";
   *   //create the NetCDF parameter class
   *   CAnyArrayParameterNetCDF anyPrm;
   *   //load an \c float array
   *   arrayPrm.loadFile((char*)fi.c_str(),var_float_array,&float_array_name);
   *   for(std::vector<float>::iterator it=var_float_array.begin();it<var_float_array.end();it++) std::cout << " " << *it;std::cout << std::endl;
   *   //load an \c string array
   *   anyPrm.loadVar(var_string_array,&string_array_name);
   *   for(std::vector<std::string>::iterator it=var_string_array.begin();it<var_string_array.end();it++) std::cout << " " << *it;std::cout << std::endl;
   * \endcode
   *
  **/
  int loadVar(std::vector<std::string> &values,std::string *var_name/*=NULL*/)
  {
#if cimg_debug>10
  std::cerr << "CParameterNetCDF::loadVar(vector<string>,\"" << ((var_name==NULL)?"default":(*var_name)) << "\")" << std::endl;
#endif
    char *ptr;
    ///get var
    if (!(var=pNCFile->get_var((*var_name).c_str()))) return NC_ERROR;
    ///get variable dimensions
    if(var->num_dims()!=2) {std::cerr << "Error in CParameterNetCDF::loadVar(vector<string>): dimension of the array should be 2 (one for array and one for string)." << std::endl;return NC_ERROR;}
    NcDim* dim_string=var->get_dim(1);
    int max_string_size=dim_string->size();
    NcDim* dim=var->get_dim(0);
    int size=dim->size();
    ///get strings
    for(int i=0;i<size;i++)
    {
      ptr=var->as_string(i*max_string_size);
//std::cerr << i+1 << "|" << ptr << "|" << std::endl;
      values.push_back(ptr);
      delete ptr;
    }
    if(values.empty()) return 1;
    return 0;
  }
  template<typename T> int loadVar(std::vector<T>&values,std::string *pVar_name/*=NULL*/)
  {
#if cimg_debug>0
  if(NcTypeStr(NcTypeInfo<T>::ncId())==NULL) {std::cerr << "Error in CParameterNetCDF::loadVar(): this NcType is not implemented yet ! see: NcTypeStr(NcTypeInfo<T>::ncId())" << std::endl;return(NC_ERROR);}
#endif
#if cimg_debug>10
  std::cerr << "CParameterNetCDF::loadVar(vector<" << (NcTypeInfo<T>::ncStr()) << ">,\"" << ((pVar_name==NULL)?"default":(*pVar_name)) << "\")" << std::endl;
#endif
//    ///set default var names in needed
//    std::string var_name;
//    if(pVar_name==NULL) pVar_name=setDefaultVarName(var_name);
    ///get var
    if (!(var=pNCFile->get_var((*pVar_name).c_str()))) {return NC_ERROR;}
    ///get variable dimension
    if(var->num_dims()!=1) {std::cerr << "Error in CParameterNetCDF::loadVar(): dimension of the array should be 1." << std::endl;return NC_ERROR;}
    NcDim* dim=var->get_dim(0);
    int size=dim->size();
    ///allocate space for data
    values.assign(size,-99);
#if cimg_debug>2
    std::cerr << "CParameterNetCDF: check" << (NcTypeInfo<T>::ncId()) << "==" << (var->type()) << std::endl;
#endif
    if( (NcTypeInfo<T>::ncId())==(var->type()) )//check if same type
    {
#if cimg_debug>10
      std::cerr << "CParameterNetCDF: get" << std::endl;
#endif
      ///get data
      if (!var->set_cur((long int)0)) {return NC_ERROR;}
      if (!var->get(&(values[0]),size)) {return NC_ERROR;}
    }
/*
    else//convert to same type
    {
#if cimg_debug>10
      std::cerr << "Information/CParameterNetCDF: convert <" << NcTypeStr(var->type()) << ">" << var_name << " to <" << (NcTypeInfo<T>::ncStr()) << ">" << std::endl;
//! \todo Warning: convert \c NcType to \c T type (actually only \c as_int is implemented)
std::cerr << "Warning/CArrayParameterNetCDF: only read T=int as only 'as_int()' is implemented !!" << std::endl;
#endif
      if (!(values=(var->as_int(0)))) return NC_ERROR;//#####
    }
*/
    return 0;
  }
  //! load attribute value
  /**
   * get value of attribute of a specified variable (using optional \c pVar_name ) or current variable (by default)
   *
   * \param attribute_name: attribute name (input)
   * \param attribute: attribute value (output)
   * \param pVar_name: name of the variable (optional input)
   *
   * \code
   *   
   * \endcode
   *
  **/
  template<typename T> int loadAttribute(std::string attribute_name,T &attribute,std::string *pVar_name=NULL)
  {
#if cimg_debug>0
  if(NcTypeStr(NcTypeInfo<T>::ncId())==NULL) {std::cerr << "Error in CParameterNetCDF::loadAttribute(): this NcType is not implemented yet ! see: NcTypeStr(NcTypeInfo<T>::ncId())" << std::endl;return(NC_ERROR);}
#endif
#if cimg_debug>10
  std::cerr << "CParameterNetCDF::loadAttribute(\"" << attribute_name << "\",<" << NcTypeStr(NcTypeInfo<T>::ncId()) << ">,\"" << ((pVar_name==NULL)?"current":(*pVar_name)) << "\")" << std::endl;
#endif
    T value;
    ///manage var name if needed
    std::string var_name;
    if(pVar_name!=NULL)
    {///check for var member is initialised
      if(var==NULL)
      {///initialise using specified name
        if (!(var=pNCFile->get_var((*pVar_name).c_str()))) return NC_ERROR;
      }
      else
      {///check for var member is valid
        if(!(var->is_valid())) return NC_ERROR;
      }
    }
    else
    {///set default var name
      //######
    }
    ///get attribute
    NcAtt *attr;
    if (!(attr=var->get_att(attribute_name.c_str()))) return NC_ERROR;
#if cimg_debug>2
    std::cerr << "CParameterNetCDF: check " << (NcTypeInfo<T>::ncId()) << "==" << (attr->type()) << std::endl;
#endif
    if( (NcTypeInfo<T>::ncId())==(attr->type()) )//check if same type
    {
#if cimg_debug>10
      std::cerr << "CParameterNetCDF: get" << std::endl;
#endif
      ///get value (i.e. equivalent to NcVar::get() for NcAtt)
      NcValues *val;
      if (!(val=attr->values())) return NC_ERROR;
      T *ptr=(T*)(val->base());
      if (attr->num_vals()!=1) std::cerr << "Warning: CParameterNetCDF::loadAttribute get only first value of the vector." << std::endl;
      value=ptr[0];
      delete val;
    }
    else//convert to same type
    {
#if cimg_debug>10
      std::cerr << "Information/CParameterNetCDF: convert <" << NcTypeStr(attr->type()) << ">" << attribute_name << " to <" << (NcTypeInfo<T>::ncStr()) << ">" << std::endl;
//! \todo Warning: convert \c NcType to \c T type (actually only \c as_int is implemented)
std::cerr << "Warning/CParameterNetCDF: only read T=int as only 'as_int()' is implemented !!" << std::endl;
#endif
      if (!(value=(attr->as_int(0)))) return NC_ERROR;//#####
    }
    attribute=value;
    return 0;
  }
  int loadAttribute(std::string attribute_name,std::string &attribute,std::string *pVar_name=NULL)
  {
#if cimg_debug>0
  if(NcTypeStr(NcTypeInfo<char>::ncId())==NULL) {std::cerr << "Error in CParameterNetCDF::loadAttribute(): this NcType is not implemented yet ! see: NcTypeStr(NcTypeInfo<T>::ncId())" << std::endl;return(NC_ERROR);}
#endif
#if cimg_debug>10
  std::cerr << "CParameterNetCDF::loadAttribute(\"" << attribute_name << "\",<string>,\"" << ((pVar_name==NULL)?"current":(*pVar_name)) << "\")" << std::endl;
std::cerr << "var is " << ((var==NULL)?"NULL":((var->is_valid())?"valid":"NOT valid")) << std::endl;
#endif
    std::string value;
    ///manage var name if needed
    std::string var_name;
    if(pVar_name!=NULL)
    {///check for var member is initialised
      if(var==NULL)
      {///initialise using specified name
        if (!(var=pNCFile->get_var((*pVar_name).c_str()))) return NC_ERROR;
      }
      else
      {///check for var member is valid
        if(!(var->is_valid())) return NC_ERROR;
      }
    }
    else
    {///set default var name
      //######
    }
    ///get attribute
    NcAtt *attr;
    if (!(attr=var->get_att(attribute_name.c_str()))) return NC_ERROR;
#if cimg_debug>2
    std::cerr << "CParameterNetCDF: check " << (NcTypeInfo<char>::ncId()) << "==" << (attr->type()) << std::endl;
#endif
    if( (NcTypeInfo<char>::ncId())==(attr->type()) )//check if same type
    {
#if cimg_debug>10
      std::cerr << "CParameterNetCDF: get" << std::endl;
#endif
      char *ptr;
      if ((ptr=(attr->as_string(0)))==NULL) return NC_ERROR;
      value=ptr;
      delete ptr;
    }
    attribute=value;
    return 0;
  }



  template<typename T> int loadAttribute(std::string attribute_name,std::vector<T> &attribute,std::string *pVar_name=NULL)
  {
#if cimg_debug>0
  if(NcTypeStr(NcTypeInfo<T>::ncId())==NULL) {std::cerr << "Error in CParameterNetCDF::loadAttribute(): this NcType is not implemented yet ! see: NcTypeStr(NcTypeInfo<T>::ncId())" << std::endl;return(NC_ERROR);}
#endif
#if cimg_debug>10
  std::cerr << "CParameterNetCDF::loadAttribute(\"" << attribute_name << "\",vector<" << NcTypeStr(NcTypeInfo<T>::ncId()) << ">,\"" << ((pVar_name==NULL)?"current":(*pVar_name)) << "\")" << std::endl;
#endif
    std::vector<T> value;
    ///manage var name if needed
    std::string var_name;
    if(pVar_name!=NULL)
    {///check for var member is initialised
      if(var==NULL)
      {///initialise using specified name
        if (!(var=pNCFile->get_var((*pVar_name).c_str()))) return NC_ERROR;
      }
      else
      {///check for var member is valid
        if(!(var->is_valid())) return NC_ERROR;
      }
    }
    else
    {///set default var name
      //######
    }
    ///get attribute
    NcAtt *attr;
    if (!(attr=var->get_att(attribute_name.c_str()))) return NC_ERROR;
#if cimg_debug>2
    std::cerr << "CParameterNetCDF: check " << (NcTypeInfo<T>::ncId()) << "==" << (attr->type()) << std::endl;
#endif
    if( (NcTypeInfo<T>::ncId())==(attr->type()) )//check if same type
    {
#if cimg_debug>10
      std::cerr << "CParameterNetCDF: get" << std::endl;
#endif
      ///get value (i.e. equivalent to NcVar::get() for NcAtt)
      NcValues *val;
      if (!(val=attr->values())) return NC_ERROR;
      T *ptr=(T*)(val->base());
      if (attr->num_vals()<2) std::cerr << "Warning: CParameterNetCDF::loadAttribute only one value in the vector." << std::endl;
      for(int i=0;i<attr->num_vals();i++)
        value.push_back(ptr[i]);
      delete val;
    }
    else//convert to same type
    {
#if cimg_debug>10
      std::cerr << "Information/CParameterNetCDF: convert <" << NcTypeStr(attr->type()) << ">" << attribute_name << " to <" << (NcTypeInfo<T>::ncStr()) << ">" << std::endl;
//! \todo Warning: convert \c NcType to \c T type (actually only \c as_int is implemented)
std::cerr << "Warning/CParameterNetCDF: only read T=int as only 'as_int()' is implemented !!" << std::endl;
#endif
      if (attr->num_vals()<2) std::cerr << "Warning: CParameterNetCDF::loadAttribute only one value in the vector." << std::endl;
      for(int i=0;i<attr->num_vals();i++)
        value.push_back(attr->as_int(0));
    }
    attribute=value;
    return 0;
  }




  int loadAttribute(std::string attribute_name,std::vector<std::string> &attribute,std::string *pVar_name=NULL)
  {
#if cimg_debug>0
  if(NcTypeStr(NcTypeInfo<char>::ncId())==NULL) {std::cerr << "Error in CParameterNetCDF::loadAttribute(): this NcType is not implemented yet ! see: NcTypeStr(NcTypeInfo<T>::ncId())" << std::endl;return(NC_ERROR);}
#endif
#if cimg_debug>10
  std::cerr << "CParameterNetCDF::loadAttribute(\"" << attribute_name << "\",vector<string>,\"" << ((pVar_name==NULL)?"current":(*pVar_name)) << "\")" << std::endl;
#endif
    std::string manyString;//many string ex.: "str1,str2,str3" or "str1 str2 str3"
    std::vector<std::string> value;//strings in vector ex.: str1 str2 str3
    int error;
    if((error=loadAttribute(attribute_name,manyString,pVar_name))!=0) return error;
    ///split the single block "manyString" to vector containing all the string
    char *cstr,*ptr;char *split=(char*)(" ,;");
#if cimg_debug>2
  std::cerr << "Information: CParameterNetCDF::loadAttribute(,vector<string>,) splitting string on any of the following chars: \"" << split << "\" (cf. strtok C++function)" << std::endl;
#endif
    cstr=new char [manyString.size()+1];
    strcpy(cstr,manyString.c_str());
    ptr=strtok(cstr,split);
    while (ptr!=NULL)
    {
      value.push_back(ptr);
      ptr=strtok(NULL,split);
    }
    delete[] cstr;
    ///
    attribute=value;
    return 0;
  }
};

#endif/*STRUCT_PARAMETER_NETCDF*/
