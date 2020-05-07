#ifndef CIMG_NETCDF
#define CIMG_NETCDF

#include <iostream>
#include <string>
#include <vector>
#include "../NetCDF/include/netcdfcpp.h"

#include "../NetCDF.Tool/NetCDFinfo.h"

#include "../CImg.Tool/useCImg.h"

#define NC_ERROR   -2
#define CODE_ERROR -3
#define DIM_ERROR  -4

#define CIMG_NETCDF_VERSION "v0.8.4"

//! add NetCDF read/write support to CImg class
/**
 * an object of this class may be created or CImg may derived from this class to handle NetCDF
 * \li object in \c .cpp
 * \code
 *   CImgNetCDF<int> object;
 * \endcode
 * \li derivation in \c .h
 * \code
 *   template<typename T> CnewClass: \c CImg<T>, \c CImgNetCDF<T>
 * \endcode
 *
 * \note \c CImgNetCDF and \c CImg must be of the same type
 * \note For this implementation, NetCDF variable dimensions need to be 5: four fixed dimensions (x,y,z,v) and one unlimited dimension (t)
 *
 * \codevpNCDim
 *   
 * \endcode
 *
 * \see CImg
 *
**/
template<typename T> class CImgNetCDF
{
  //variable
 public:
  //! pointer to NetCDF error handling
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
  //! pointer to NetCDF file
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
  NcFile *pNCFile;bool pNCFile_init_here;
  //! pointers to NetCDF dimensions
  /**
   * 
   *
   * \code
   *   if (!(pDimi=dataFile.get_dim("dim_x"))) return NC_ERROR;//Read
   *   if (!(pDimi=dataFile.add_dim("dim_x",dimi))) return NC_ERROR;//Write
   * \endcode
   *
   * \see loadDimx,y,z,v LOAD_DIM
   *
   **/
  std::vector<NcDim*>vpNCDim;bool vpNCDim_init_here;
  //! virtual functions which may return CImg dimensions
  /**
   * To use this functions, CImg may derive from this class.
   *
   * \code
   *   dataFile.width()
   * \endcode
   *
   * \see loadDimTime loadDimx,y,z,v from LOAD_DIM checkDimx,y,z,v(CImg) from CHECK_DIM
   *
   **/

  //std::cerr << "CImgNetCDF::__dim__()" << d <<std::endl;


#define DIM(X) int dim##X(void){return 0;}
  DIM(x)
  DIM(y)
  DIM(z)
  DIM(v)

  int dim(int d)
  {
    switch(d)
    {
      case 0: return dimx();break;
      case 1: return dimy();break;
      case 2: return dimz();break;
      case 3: return dimv();break;
    }//switch
    return -99;
  }//dim

  //! functions read dimensions from NetCDF file
  /**
   * 
   *
   * \code
   *   dataFile.loadwidth()
   * \endcode
   *
   * \see loadDimTime DIM CHECK_DIM
   *
   **/

  //std:cerr << "CImgNetCDF::" << __func__ << "()" << std::endl;
  
#define LOAD_DIM(X,N) int loadDim##X(void)	\
  {						\
    if(N>=vpNCDim.size()) return CODE_ERROR;	\
    return (int)vpNCDim[N]->size();		\
  }
  LOAD_DIM(x,0)
  LOAD_DIM(y,1)
  LOAD_DIM(z,2)
  LOAD_DIM(v,3)

    //                 std::cerr << "CImgNetCDF::" << __func__ << "()" << std::endl;

  int loadDim(int d)
  {
    if(d>=(int)vpNCDim.size()) return CODE_ERROR;
    return (int)vpNCDim[d]->size();
  }//loadDim
  //! 
  /**
   * 
   *
   * \code
   *   if (!(pDimt=dataFile.get_dim("time"))) return NC_ERROR;//Read
   *   if (!(pDimt=dataFile.add_dim("time"))) return NC_ERROR;//Write: add an unlimited dimension
   * \endcode
   *
   * \note if \c pNCDimt==NULL , the class should behave as there is no unlimited direction.
   *
   * \see loadDimTime
   *
   **/
  NcDim *pNCDimt;
  //! pointers to NetCDF data
  /**
   * access to NetCDF data
   *
   * \code
   *   
   * \endcode
   *
   **/
  NcVar *pNCvar;
  //! function read unlimited dimension from NetCDF file
  /**
   * the unlimited time dimension may be a loaded one by one
   *
   * \code
   *   dataFile.loadDimTime()
   * \endcode
   *
   * \see loadDimx,y,z,v from LOAD_DIM
   *
   **/
  int loadDimTime(void)
  {
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "()" << std::endl;
#endif
    if(pNCDimt==NULL) {std::cerr << "CImgNetCDF::" << __func__ << "() error: pNCDimt==NULL." << std::endl;return CODE_ERROR;}
    return (int)pNCDimt->size();
  }//loadDimTime
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
  CImgNetCDF(void)
    {
#if cimg_debug>10
      std::cerr << "CImgNetCDF::" << __func__ << "()" << std::endl;
#endif
      pNCError=new NcError(NcError::verbose_nonfatal);
      pNCFile=NULL;pNCFile_init_here=false;
      /*vpNCDimv=NULL;*/vpNCDim_init_here=false;
      pNCDimt=NULL;
      pNCvar=NULL;
    }//constructor
  virtual ~CImgNetCDF(void)
    {
#if cimg_debug>10
      std::cerr << "CImgNetCDF::" << __func__ << "()" << std::endl;
#endif
      //delete(pNCvar);
      if(!vpNCDim_init_here) 
	/*for(int i=0;i<vpNCDim.size();i++) delete(vpNCDim[i]); else*/
	for(int i=0;i<(int)vpNCDim.size();i++) vpNCDim[i]=NULL;
      if(pNCFile_init_here) {
    	pNCFile->close(); 
    	delete(pNCFile); 
      }
      else pNCFile=NULL;
      delete(pNCError);
    }//destructor
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
  int loadNetCDFFile(char *fileName)//,std::vector<std::string> pDim_names=NULL,std::string *pTime_name=NULL)
  {
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "(" << fileName << "," << std::endl;//;
    //  std::cerr << "," << ((pTime_name==NULL)?"default":(*pTime_name).c_str()) << ")" << std::endl;
#endif
    pNCFile=new NcFile(fileName,NcFile::ReadOnly);
    pNCFile_init_here=true;
    //    if(pDim_names==NULL)
    return !(pNCFile->is_valid());
    //    else
    //      return loadNetCDFDims(*pDim_names,pTime_name);
  }//loadNetCDFFile
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
  int saveNetCDFFile(char *fileName, std::vector<std::string> *pDim_names=NULL,std::string *pTime_name=NULL)
  {
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "(" << fileName << ",";
    //  std::cerr << ((dimi_name==NULL)?"default":dimi_name) << "," << ((dimj_name==NULL)?"default":dimj_name) << ",";
    std::cerr << ((pTime_name==NULL)?"":*pTime_name) << ")" << std::endl;
#endif
    pNCFile=new NcFile(fileName,NcFile::Replace);// Write
    pNCFile_init_here=true;
    if( (pDim_names!=NULL) && (pTime_name!=NULL)) return addNetCDFDims(*pDim_names,*pTime_name);
    else return 0;
  }//saveNetCDFFile
  int saveNetCDFFile(char *fileName,NcFile::FileMode fmod, std::vector<std::string> *pDim_names=NULL,std::string *pTime_name=NULL)
  {
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "(" << fileName << ",";
    //  std::cerr << ((dimi_name==NULL)?"default":dimi_name) << "," << ((dimj_name==NULL)?"default":dimj_name) << ",";
    std::cerr << ((pTime_name==NULL)?"":*pTime_name) << ")" << std::endl;
#endif
    pNCFile=new NcFile(fileName,fmod);//Replace Write
    pNCFile_init_here=true;
    if( (pDim_names!=NULL) && (pTime_name!=NULL)) return addNetCDFDims(*pDim_names,*pTime_name);
    else return 0;
  }//saveNetCDFFile
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
  int setNetCDFFile(NcFile *pNetCDFFile)
  {
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "(" << pNetCDFFile << ")" << std::endl;
#endif
    pNCFile=pNetCDFFile;
    pNCFile_init_here=false;
    return 0;
  }//setNetCDFFile
  
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
  NcFile *getNetCDFFile(){
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "(" << pNetCDFFile << ")" << std::endl;
#endif
    return pNCFile;
  }//getNetCDFFile
  
  //! add variables in NetCDF file
  /**
   * save variables in NetCDF file, may be called after creation of dimensions \c CImgNetCDF::addNetCDFDims
   *
   * \code
   *   dimv=2;char *fileName="2D2C.nc";
   *   C2DnCNetCDFvelocity vel;//this class MUST be derivated as dimi(), dimj() and spectrum() are VIRTUAL
   *   vel.pNCFile=new NcFile(fileName,NcFile::Replace);
   *   vel.addNetCDFDims("dimX","dimY","time");
   *   std::vector<std::string> var_names(dimv+1);
   *   var_names[0]="vel_X";var_names[1]="vel_Y";
   *   var_names[2]="flag";
   *   std::vector<std::string> unit_names(2);
   *   unit_names[0]="m/s";unit_names[1]="none";
   *   addNetCDFVar(&var_names,&unit_names);
   * \endcode
   *
   * \see CImgNetCDF::addNetCDFDims
   *
   **/
  int addNetCDFVar(std::string var_name,std::string unit_name)
  {
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "(" << var_name << "," << unit_name << ")" << std::endl;
#endif
    if(pNCDimt==NULL)
      {
#if cimg_debug>10
	std::cerr<<__func__<<" no unlimited"<<std::endl;
#endif
	switch(vpNCDim.size())
	  {
	  case 4:if (!(pNCvar=pNCFile->add_var(var_name.c_str(),(NcType)NcTypeInfo<T>::ncId(),vpNCDim[3],vpNCDim[2],vpNCDim[1],vpNCDim[0]))) return NC_ERROR;break;
	  case 3:if (!(pNCvar=pNCFile->add_var(var_name.c_str(),(NcType)NcTypeInfo<T>::ncId(),vpNCDim[2],vpNCDim[1],vpNCDim[0]))) return NC_ERROR;break;
	  case 2:if (!(pNCvar=pNCFile->add_var(var_name.c_str(),(NcType)NcTypeInfo<T>::ncId(),vpNCDim[1],vpNCDim[0]))) return NC_ERROR;break;
	  case 1:if (!(pNCvar=pNCFile->add_var(var_name.c_str(),(NcType)NcTypeInfo<T>::ncId(),vpNCDim[0]))) return NC_ERROR;break;
	    //default:NULL;
	  }
      }
    else
      {
#if cimg_debug>10
	std::cerr<<__func__<<" unlimited"<<std::endl;
#endif
	switch(vpNCDim.size())
	  {
	  case 4:if (!(pNCvar=pNCFile->add_var(var_name.c_str(),(NcType)NcTypeInfo<T>::ncId(),pNCDimt,vpNCDim[3],vpNCDim[2],vpNCDim[1],vpNCDim[0]))) return NC_ERROR;break;
	  case 3:if (!(pNCvar=pNCFile->add_var(var_name.c_str(),(NcType)NcTypeInfo<T>::ncId(),pNCDimt,vpNCDim[2],vpNCDim[1],vpNCDim[0]))) return NC_ERROR;break;
	  case 2:if (!(pNCvar=pNCFile->add_var(var_name.c_str(),(NcType)NcTypeInfo<T>::ncId(),pNCDimt,vpNCDim[1],vpNCDim[0]))) return NC_ERROR;break;
	  case 1:if (!(pNCvar=pNCFile->add_var(var_name.c_str(),(NcType)NcTypeInfo<T>::ncId(),pNCDimt,vpNCDim[0]))) return NC_ERROR;break;	
	    //default:NULL;
	  }
      }
    //Define units attributes for data variables.
    if (!(pNCvar->add_att("units",unit_name.c_str()))) return NC_ERROR;
    return 0;
  }//addNetCDFVar
  //! add variables in NetCDF file
  /**
   * save variables in NetCDF file, may be called after creation of dimensions \c CImgNetCDF::addNetCDFDims
   *
   * \code
   *   dimv=2;char *fileName="2D2C.nc";
   *   C2DnCNetCDFvelocity vel;//this class MUST be derivated as dimi(), dimj() and spectrum() are VIRTUAL
   *   vel.pNCFile=new NcFile(fileName,NcFile::Replace);
   *   vel.addNetCDFDims("dimX","dimY","time");
   *   std::vector<std::string> var_names(dimv+1);
   *   var_names[0]="vel_X";var_names[1]="vel_Y";
   *   var_names[2]="flag";
   *   std::vector<std::string> unit_names(2);
   *   unit_names[0]="m/s";unit_names[1]="none";
   *   addNetCDFVar(&var_names,&unit_names);
   * \endcode
   *
   * \see CImgNetCDF::addNetCDFDims
   *
   **/


  //  int addNetCDFVar(CImg &img,std::string var_name,std::string unit_name,std::vector<std::string> dim_names,std::string time_name)
  int addNetCDFVar(CImg<T> &img,std::string var_name,std::string unit_name)
  {
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "(CImg<" << img.pixel_type() << ">," << var_name << "," << unit_name << ")" << std::endl;
#endif
    //check dims
    for(int i=0;i<(int)vpNCDim.size();i++)
      {
	switch(i){
        case 3:if(loadDim(i)!=img.spectrum()) {return DIM_ERROR;}break;
        case 2:if(loadDim(i)!=img.depth()) {return DIM_ERROR;}break;
        case 1:if(loadDim(i)!=img.height()) {return DIM_ERROR;}break;
        case 0:if(loadDim(i)!=img.width()) {return DIM_ERROR;}break;	
	  //default:NULL;
	}
      }
    return addNetCDFVar(var_name,unit_name);
  }//addNetCDFVar


  //! add variable data in NetCDF file
  /**
   * add variable data in NetCDF file, may be called after creation of both dimensions \c CImgNetCDF::addNetCDFDims and variables \c CImgNetCDF::addNetCDFVar 
   *
   * \code
   *   
   * \endcode
   *
   * \see CImgNetCDF::addNetCDFDims CImgNetCDF::addNetCDFVar
   *
   **/
  //for CImg inheritance ###
  /*  template<typename T> T* ptr(void)
      {
      return NULL;
      }*/
  int addNetCDFData(int time=-1){
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "()" << std::endl;
#endif
    if(pNCvar==NULL) return CODE_ERROR;
    T *ptr=NULL/*(*this).ptr();*/;
    if(pNCDimt==NULL)
      {//no unlimited direction
	//write data
	if (!pNCvar->put(ptr/*,0(*this).size()*/)) return NC_ERROR;
      }
    else
      {
	//set time if needed (append mode)
	if(time==-1) time=loadDimTime();
	std::cerr<<__func__<<" unlimited direction="<<time<<std::endl;
	//write data
	if (!pNCvar->put_rec(ptr,time)) return NC_ERROR;
      }
    return 0;
  }//addNetCDFData
  //for CImg as function parameter
  int addNetCDFData(CImg<T> &img,int time=-1){
#if cimg_debug>10
    std::cerr<<"CImgNetCDF::"<< __func__<<"(CImg<"<<img.pixel_type()<<">";
    if(pNCDimt==NULL) std::cerr<<")"<< std::endl;
    else std::cerr<<","<<time<<" (if -1 append time))"<<std::endl;
#endif
    if(pNCvar==NULL) return CODE_ERROR;
    for(int a = 0 ; a<(int)vpNCDim.size();a++){
      switch(a){
      case 3:if(loadDim(a)!=img.spectrum()) {return DIM_ERROR;}break;
      case 2:if(loadDim(a)!=img.depth()) {return DIM_ERROR;}break;
      case 1:if(loadDim(a)!=img.height()) {return DIM_ERROR;}break;
      case 0:if(loadDim(a)!=img.width()) {return DIM_ERROR;}break;	
      default:return CODE_ERROR;
      }
    }
    //write data
    T *ptr=img.data();
    if(pNCDimt==NULL)
      {//no unlimited direction
#if cimg_debug>10
	std::cerr<<__func__<<" no unlimited direction"<<std::endl;
#endif
	//write data
	switch(vpNCDim.size())
          {
	  case 1:if (!pNCvar->put(ptr,img.size())) return NC_ERROR;break;
	  case 2:if (!pNCvar->put(ptr,img.height(),img.width())) return NC_ERROR;break;
	  case 3:if (!pNCvar->put(ptr,img.depth(),img.height(),img.width())) return NC_ERROR;break;
	  case 4:if (!pNCvar->put(ptr,img.spectrum(),img.depth(),img.height(),img.width())) return NC_ERROR;break;
	  default:return CODE_ERROR;
          }
      }
    else
      {//use unlimited direction
	//set time if needed (append mode)
	if(time==-1) time=loadDimTime();
#if cimg_debug>10
	std::cerr<<__func__<<" unlimited direction="<<time<<std::endl;
#endif
	//write data
	if (!pNCvar->put_rec(ptr,time)) return NC_ERROR;
      }
    return 0;
  }//addNetCDFData



  //! load variable dimension from NetCDF file
  /**
   * load from NetCDF file a certain number of variables from names, that will set the variable dimension
   *
   * \code
   *   
   * \endcode
   *
   **/
  int loadNetCDFVar(CImg<T> &img,std::string &var_name,std::string &pUnit_name=NULL)
  {
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "(CImg,string,string)" << std::endl;
#endif
    ///get variable
    if (!(pNCvar=pNCFile->get_var(var_name.c_str()))) return NC_ERROR;
    ///set CImg dimensions if needed
    if(img.is_empty()){
      switch(vpNCDim.size()){
      case 1:img.assign(loadDim(0));break;
      case 2:img.assign(loadDim(0),loadDim(1));break;
      case 3:img.assign(loadDim(0),loadDim(1),loadDim(2));break;
      case 4:img.assign(loadDim(0),loadDim(1),loadDim(2),loadDim(3));break;
	//default:NULL;
      }
    }
    return 0;
  }//loadNetCDFVar

  //! load dimensions from NetCDF file
  /**
   * load variable dimensions from NetCDF file
   *
   * \param  : 
   *
   * \code
   *   
   * \endcode
   *
   **/
  int loadNetCDFDims(std::vector<std::string> dim_names,std::string *pTime_name=NULL)
  {
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "(";
    std::cerr << "," << ((pTime_name==NULL)?"default":(*pTime_name).c_str()) << ")" << std::endl;
#endif
    //check requested number of dimensions //###
    if(dim_names.size()!=4) std::cerr << "CImgNetCDF::" << __func__ << "(" << std::endl;
    NcDim*pd;
    for(unsigned int a = 0 ; a<dim_names.size();a++){
      if(!(pd=pNCFile->get_dim(dim_names[a].c_str())))return NC_ERROR;//Read
      else vpNCDim.push_back(pd);//Read
    }
    if(pTime_name!=NULL)
      if (!(pNCDimt=pNCFile->get_dim((*pTime_name).c_str())))        return NC_ERROR;//Read
    //dims can be now loaded using loadDim[x,y,z,v,Time]() methods
    return 0;
  }//loadNetCDFDims





  //! load dimension names of a variable from NetCDF file
  /**
   * load variable names of a named variable from NetCDF file
   *
   * \param[in] var_name: name of the variable
   * \param[out] dim_names: names of variable dimensions (i.e. fixed dimensions)
   * \param[out] pTime_name: name of variable unlimited dimension (i.e. append dimension)
   *
   * \code
   *   
   * \endcode
   *
   **/
  int loadNetCDFDimNames(std::string &var_name,std::vector<std::string> &dim_names,std::string *pTime_name=NULL)
  {
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "(";
    std::cerr << "," << ((pTime_name==NULL)?"don't ":"") <<"search for unlimited dimension"<< ")" << std::endl<<std::flush;
#endif
//std::cerr << "CImgNetCDF::" << __func__;
    int i=0;
    NcDim*pNCDimtemp=NULL;
    ///get variable
    if (!(pNCvar=pNCFile->get_var(var_name.c_str()))) return NC_ERROR;
//std::cerr << "/get time dim="<<pNCvar->num_dims();
    //get dims
    if(pTime_name!=NULL){
      pTime_name->clear();
      if (!(pNCDimt=pNCvar->get_dim(i++)))return NC_ERROR;//Read
      *pTime_name=std::string(pNCDimt->name());
    }
//std::cerr << "/get dims="<<i;
    for(;i<pNCvar->num_dims();i++){
      if (!(pNCDimtemp=pNCvar->get_dim(pNCvar->num_dims()-i-1))) return NC_ERROR;//Read
      vpNCDim.push_back(pNCDimtemp);
    }
//std::cerr << "/ok="<<i<<"=="<<vpNCDim.size();
    dim_names.clear();
    for(unsigned int a=0;a<vpNCDim.size();a++){//std::cout<<"ess "<<vpNCDim[a]->name()<<std::endl;
      dim_names.push_back(std::string(vpNCDim[a]->name()));
    }
    //#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << " information: dim_names=";
    for(std::vector<std::string>::iterator it=dim_names.begin();it!=dim_names.end();it++) std::cerr<<"\""<<*it<<"\", ";
    std::cerr << ((pTime_name==NULL)?"no unlimited dimension as been searched":"time_name=\""+*pTime_name+"\" (unlimited)") <<std::endl<<std::flush;
    //#endif
    return 0;
  }//loadNetCDFDimNames
  //(std::string)this <<



  //! check if NetCDF \c variable and \c CImg dims are the same
  /**
   * check if NetCDF \c variable and \c CImg dims are the same
   *
   * \arg img: \c CImg image dimensions to check
   *
   * \code
   *   CImg<int> img(1,2,3,4);CImgNetCDF<int> fp("data.nc");
   *   if( fp.checkDimx(img) || fp.checkDimy(img) || fp.checkDimz(img) || fp.checkDimv(img) ) exit(1);
   * \endcode
   *
   * \see DIM LOAD_DIM
   *
   **/
  int checkDim(int d,CImg<T> img){
    std::cerr << "CImgNetCDF::" << __func__ << "(CImg)" << std::endl;
    if(d>=vpNCDim.size()) return CODE_ERROR;
    switch(d){
    case 0: return img.width()!=(int)vpNCDim[d]->size();break;
    case 1: return img.height()!=(int)vpNCDim[d]->size();break;
    case 2: return img.depth()!=(int)vpNCDim[d]->size();break;
    case 3: return img.spectrum()!=(int)vpNCDim[d]->size();break;
      //default:NULL;
    }
  }//checkDim

#define CHECK_DIM(X,N) int checkDim##X(CImg<T> img)			\
  {									\
    std::cerr << "CImgNetCDF::" << __func__ << "(CImg)" << std::endl;	\
    if(N>=vpNCDim.size()) return CODE_ERROR;				\
    return img.dim##X()!=(int)vpNCDim[N]->size();			\
  }
  CHECK_DIM(x,0)
  CHECK_DIM(y,1)
  CHECK_DIM(z,2)
  CHECK_DIM(v,3)


    //! load variable data from NetCDF file
    /**
     * load from NetCDF file variable data at a time
     *
     * \arg img: load data values in this \c CImg
     * \arg time: get data at time \c time (must be in the range 0 to < \c loadDimTime())
     *
     * \code
     *   
     * \endcode
     *
     **/
    /*
      int loadNetCDFData(CImg<T> &img,int time=0)
      {
      #if cimg_debug>10
      std::cerr << "CImgNetCDF::" << __func__ << "(CImg,time=" << time << ")" << std::endl;
      #endif
      ///get component data
      if (!pNCvar->set_cur(time,0,0,0,0)) return NC_ERROR;
      switch(vpNCDim.size()){
      case 1:if (!pNCvar->get(img.ptr(0,0,0,0), 1,loadDim(0))) return NC_ERROR;break;
      case 2:if (!pNCvar->get(img.ptr(0,0,0,0), 1,loadDim(1),loadDim(0))) return NC_ERROR;break;
      case 3:if (!pNCvar->get(img.ptr(0,0,0,0), 1,loadDim(2),loadDim(1),loadDim(0))) return NC_ERROR;break;
      case 4:if (!pNCvar->get(img.ptr(0,0,0,0), 1,loadDim(3),loadDim(2),loadDim(1),loadDim(0))) return NC_ERROR;break;
      default:NULL;
      }
      return 0;
      }*/
    int loadNetCDFData(CImg<T> &img,int time=0)
    {
      //#if cimg_debug>10
      std::cerr << "CImgNetCDF::" << __func__ << "(CImg,time=" << time << ")" << std::endl;
      //#endif
      //! \todo vector dim, so set_cur will work
      //	long int dim[3]={time,0,0};
      long int dim[5]={time,0,0,0,0};
      //cout<<"TESTTAILLE"<<vpNCDim.size()<<endl;
      ///get component data
      if (!pNCvar->set_cur(dim)) return NC_ERROR;		//PB -> return NC-ERROR
      dim[0]=1;//{1,0,0,0,0};
      int a,i;
      for(i=(int)vpNCDim.size()-1, a=0;i>=0;i--)dim[a++]=loadDim(i);
      if (!pNCvar->get(img.data(), dim)) return NC_ERROR;//*/
	
      return 0;
    }//loadNetCDFData
  int loadNetCDFData3D(CImg<T> &img,int time=0){
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "(CImg,time=" << time << ")" << std::endl;
#endif
    //! \todo vector dim, so set_cur will work
    //	long int dim[3]={time,0,0};
    long int dim[5]={time,0,0,0,0};
    //cout<<"TESTTAILLE"<<vpNCDim.size()<<endl;
    ///get component data
    //if (!pNCvar->set_cur(dim)) return NC_ERROR;		//PB -> return NC-ERROR
    pNCvar->set_rec(time);
    //cout<<"TEST\n";
    /*switch(vpNCDim.size()){
      case 1:if (!pNCvar->get(img.ptr(0,0,0,0), 1,loadDim(0))) return NC_ERROR;break;
      case 2:if (!pNCvar->get(img.ptr(0,0,0,0), 1,loadDim(1),loadDim(0))) return NC_ERROR;break;
      case 3:if (!pNCvar->get(img.ptr(0,0,0,0), 1,loadDim(2),loadDim(1),loadDim(0))) return NC_ERROR;break;
      case 4:if (!pNCvar->get(img.ptr(0,0,0,0), 1,loadDim(3),loadDim(2),loadDim(1),loadDim(0))) return NC_ERROR;break;
      default:NULL;
      }//*/
    dim[0]=1;//{1,0,0,0,0};
    //	dim[0]=loadDim(3);
    dim[1]=loadDim(2);
    dim[2]=loadDim(1);
    dim[3]=loadDim(0);
    //dim[4]=loadDim(0);
    //	int a,i;
    //	for(i=vpNCDim.size()-1, a=1;i>=0;i--)dim[a++]=loadDim(i);
    if (!pNCvar->get(img.ptr(0,0,0,0), dim)) return NC_ERROR;//*/
	
    return 0;
  }//loadNetCDFData3D
  int loadNetCDFData2D(CImg<T> &img,int time=0)
  {
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "(CImg,time=" << time << ")" << std::endl;
#endif
    //! \todo vector dim, so set_cur will work
    //	long int dim[3]={time,0,0};
    long int dim[3]={time,0,0};
    //cout<<"TESTTAILLE"<<vpNCDim.size()<<endl;
    ///get component data
    if (!pNCvar->set_cur(dim)) return NC_ERROR;		//PB -> return NC-ERROR
    //cout<<"TEST\n";
    /*switch(vpNCDim.size()){
      case 1:if (!pNCvar->get(img.ptr(0,0,0,0), 1,loadDim(0))) return NC_ERROR;break;
      case 2:if (!pNCvar->get(img.ptr(0,0,0,0), 1,loadDim(1),loadDim(0))) return NC_ERROR;break;
      case 3:if (!pNCvar->get(img.ptr(0,0,0,0), 1,loadDim(2),loadDim(1),loadDim(0))) return NC_ERROR;break;
      case 4:if (!pNCvar->get(img.ptr(0,0,0,0), 1,loadDim(3),loadDim(2),loadDim(1),loadDim(0))) return NC_ERROR;break;
      default:NULL;
      }//*/
    dim[0]=1;//{1,0,0,0,0};
    int a,i;
    for(i=(int)vpNCDim.size()-1, a=1;i>=0;i--)dim[a++]=loadDim(i);
    //if (!pNCvar->get(img.ptr(0,0,0,0), dim)) return NC_ERROR;//*/
    if (!pNCvar->get_rec( )) return NC_ERROR;
    return 0;
  }//loadNetCDFData2D


  //! same as previously but without time
  /**
   * 
   **/
  int loadNetCDFDimNamesNoUnlimited(std::string &var_name,std::vector<std::string> &dim_names)
  {
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "(";
#endif
    int i=1;
    NcDim*pNCDimtemp=NULL;
    
    ///get variable
    if (!(pNCvar=pNCFile->get_var(var_name.c_str()))) return NC_ERROR;
    //get dims
    for(;i<=pNCvar->num_dims();i++){
      if (!(pNCDimtemp=pNCvar->get_dim(pNCvar->num_dims()-i))){
	return NC_ERROR;}//Read
      vpNCDim.push_back(pNCDimtemp);
    }
    
    dim_names.clear();
    for(unsigned int a=0;a<vpNCDim.size();a++){//std::cout<<"ess "<<vpNCDim[a]->name()<<std::endl;
      dim_names.push_back(std::string(vpNCDim[a]->name()));
    }
    //#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << " information: dim_names=";
    for(std::vector<std::string>::iterator it=dim_names.begin();it!=dim_names.end();it++) std::cerr<<"\""<<*it<<"\", ";
    //#endif
    return 0;
  }//loadNetCDFDimNamesNoUnlimited
  
  
  //for CImg inheritance ###
  int addNetCDFDims(std::vector<std::string> dim_names,std::string time_name)
  {
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "(";
    //std::cerr << ((dim_names==NULL)?"default":dimi_name) << "," << ((dimj_name==NULL)?"default":dimj_name) << ",";
    std::cerr << time_name << ")" << std::endl;
#endif
    vpNCDim.clear();
    NcDim* pd;
    for(int a=0;a<(int)dim_names.size();a++)
      {
	if(!(pd=pNCFile->add_dim(dim_names[a].c_str(),dim(a)))) return NC_ERROR;//Write: add a dimension
	vpNCDim.push_back(pd);
      }
    if(time_name.empty())
      pNCDimt=NULL;
    else
      if(!(pNCDimt=pNCFile->add_dim(time_name.c_str()/*unlimited*/))) return NC_ERROR;//Write: add an unlimited dimension
    vpNCDim_init_here=true;
    return 0;
  }//addNetCDFDims
  //! \overload without unlimited direction
  int addNetCDFDims(std::vector<std::string> dim_names)
  {
    std::string time_name;
    return addNetCDFDims(dim_names,time_name);
  }//addNetCDFDims
  //! add one dimension checking for its availability
  /**
   * use existing dimension or create one
   * \note This is not the function for \c unlimited dimension.
   * \param [in]dim_name: name of the dimension
   * \param [in]size: value of the dimension (i.e. size)
   * \param [out]pDim: NetCDF pointer to the dimension (i.e. existing or new dimension)
   * \return OK=0 or NetCDF error: DIM_ERROR (dimension all ready exists, but size is different) or NC_ERROR (output error).
   **/
  int addNetCDFDim(std::string &dim_name,int size,NcDim* &pDim)
  {
/*
    std::cerr<<__FILE__<<"/"<<__func__<<"("<<std::flush;
    std::cerr<<"dim="<<dim_name<<", size="<<size<<std::flush;
    std::cerr<<", pDim="<<dim_name<<")"<<std::endl<<std::flush;
*/
    //check availability
    if(pDim==pNCFile->get_dim(dim_name.c_str()))
      {//check size of the existing dimension
	if(pDim->size()!=size)
	  {
	    std::cerr<<"Warning: using existing \""<<dim_name<<"\" dimension, which differs in size (i.e. "<<pDim->size()<<"."<<std::endl;
	    return DIM_ERROR;
	  }
      }
    else
      {//create new dimension
	std::cerr<<"Information: create new \""<<dim_name<<"\" dimension."<<std::endl<<std::flush;
	if(!(pDim=pNCFile->add_dim(dim_name.c_str(),size))) return NC_ERROR;
      }
    return 0;
  }//addNetCDFDim
  //for CImg as function parameter
  int addNetCDFDims(CImg<T> &img,std::vector<std::string> dim_names,std::string time_name)
  {
//#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "(CImg<" << img.pixel_type() << ">,"<<std::flush;
    std::cerr << "dims=";
    for(int i=0;i<dim_names.size();++i) std::cerr <<dim_names[i]<< ",";
    //std::cerr << ((dim_names==NULL)?"default":dimi_name) << "," << ((dimj_name==NULL)?"default":dimj_name) << ",";
    std::cerr<<"  "<< time_name << ")" << std::endl<<std::flush;
//#endif
    int error;
    NcDim* pd;
    for(int i=0;i<(int)dim_names.size();i++)
      {
	switch(i){//Write or use existing
        case 0:if((error=addNetCDFDim(dim_names[i],img.width(),   pd))!=0) return error;break;
        case 1:if((error=addNetCDFDim(dim_names[i],img.height(),  pd))!=0) return error;break;
        case 2:if((error=addNetCDFDim(dim_names[i],img.depth(),   pd))!=0) return error;break;
        case 3:if((error=addNetCDFDim(dim_names[i],img.spectrum(),pd))!=0) return error;break;
	  //default:NULL;
	}
	vpNCDim.push_back(pd);
      }
    if(time_name.empty())
      {
#if cimg_debug>10
	std::cerr<<__func__<<"/time_name.empty()"<< std::endl;
#endif
	pNCDimt=NULL;
      }
    else
      {
#if cimg_debug>10
	std::cerr<<__func__<<"/time_name.!empty()"<< std::endl;
#endif
	if(!(pNCDimt=pNCFile->add_dim(time_name.c_str()/*unlimited*/))) return NC_ERROR;//Write: add an unlimited dimension
      }
    vpNCDim_init_here=true;
    return 0;
  }//addNetCDFDims
  //! \overload without unlimited direction
  int addNetCDFDims(CImg<T> &img,std::vector<std::string> dim_names)
  {
    std::string time_name;
    return addNetCDFDims(img,dim_names,time_name);
  }//addNetCDFDims
  //! for CImg inheritance
  int setNetCDFDims(std::string &dim_names,NcDim *pNetCDFDimt=NULL)
  {
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "(";
    //  std::cerr << ((dim_names==NULL)?"default":dimi_name) << "," << ((dimj_name==NULL)?"default":dimj_name) << ",";
    std::cerr << time_name << ")" << std::endl;
#endif
    vpNCDim.clear();
    NcDim*pd;
    if(!(pd=pNCFile->add_dim(dim_names.c_str(),dim_names.size())))return NC_ERROR;//Read
    else{vpNCDim.push_back(pd);}
    pNCDimt=pNetCDFDimt;
    return 0;
  }//setNetCDFDims
  //! 
  /**
   * 
   **/
  int loadNetCDFDataNoUnlimited(CImg<T> &img)
  {
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "(CImg)" << std::endl;
#endif
    switch(vpNCDim.size()){
    case 1:if (!pNCvar->get(img.ptr(0,0,0,0), loadDim(0))) return NC_ERROR;break;
    case 2:if (!pNCvar->get(img.ptr(0,0,0,0), loadDim(1),loadDim(0))) return NC_ERROR;break;
    case 3:if (!pNCvar->get(img.ptr(0,0,0,0), loadDim(2),loadDim(1),loadDim(0))) return NC_ERROR;break;
    case 4:if (!pNCvar->get(img.ptr(0,0,0,0), loadDim(3),loadDim(2),loadDim(1),loadDim(0))) return NC_ERROR;break;
      //default:NULL;
    }
    return 0;
  }//loadNetCDFDataNoUnlimited
  
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
  int setNetCDFDims(std::vector<NcDim*> VDim,NcDim *pNetCDFDimt=NULL)
  {
#if cimg_debug>10
    std::cerr << "CImgNetCDF::" << __func__ << "(" ;
    for(unsigned int a=0;a<VDim.size();a++){
      std::cerr << VDim[a]<< ",";
      std::cerr << pNetCDFDimt << ")" << std::endl;
#endif
      vpNCDim.clear();
      for(int a=0;a<(int)VDim.size();a++){
	vpNCDim.push_back(VDim[a]);
      }
      pNCDimt=pNetCDFDimt;
      vpNCDim_init_here=false;
      return 0;
    }//setNetCDFDims
  };//CImgNetCDF




  //! test inheritance for CImgNetCDF class
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

  template<typename T> class CImgNetCDF_test:public CImgNetCDF<T>
  {
  public:

#ifdef DIM
#undef DIM
#endif
#define DIM(X,N) int dim##X(void){		\
      return (CImgNetCDF<T>::vpNCDim[N]);	\
    }
    DIM(x,0)
    DIM(y,1)
    DIM(z,2)
    DIM(v,3)


    //call C2DNetCDF::saveNetCDFFile
    int saveNetCDFFile(char *fileName,std::vector<std::string>dim_names,std::string time_name)
    {
#if cimg_debug>10
      std::cerr << "CImgNetCDF_test::" << __func__ << "(" << fileName << "," ;
      std::cerr << time_name << ")" << std::endl;
#endif
      return CImgNetCDF<T>::saveNetCDFFile(fileName,&dim_names,&time_name);
    }




  //! add variable data in NetCDF file
  /**
   * add variable data in NetCDF file, may be called after creation of both dimensions \c CImgNetCDF::addNetCDFDims and variables \c CImgNetCDF::addNetCDFVar 
   *
   * \code
   *   
   * \endcode
   *
   * \see CImgNetCDF::addNetCDFDims CImgNetCDF::addNetCDFVar
   *
   */
  int addNetCDFData(void){
#if cimg_debug>10
    std::cerr << "CImgNetCDF_test::" << __func__ << "()" << std::endl;
#endif
    if((*this).pNCvar==NULL) return CODE_ERROR;
    int time=CImgNetCDF<T>::loadDimTime();//vpNCDim[2] << "," << vpNCDim[3]
    //make dummy data
    int dim=1;
    for(unsigned int i=0 ; i < (CImgNetCDF<T>::vpNCDim.size());i++)
      dim=dim*(CImgNetCDF<T>::vpNCDim[i]);
    T *ptr=(T*)malloc(sizeof(T)*dim);
    for(int i=0;i<dim;i++) ptr[i]=(T)i;
    //write
    if (!((*this).pNCvar)->put_rec(ptr,time)) return NC_ERROR;
    return 0;
  }





  //! call CImgNetCDF::loadFile
  int loadFile(char *fileName,char *dimi_name=NULL,char *dimj_name=NULL,char *time_name=NULL)
  {
#if cimg_debug>10
    std::cerr << "CImgNetCDF_test::" << __func__ << "(" << fileName << "," << ((dimi_name==NULL)?"default":dimi_name) << "," << ((dimj_name==NULL)?"default":dimj_name) << "," << ((time_name==NULL)?"default":time_name) << ")" << std::endl;
#endif
    CImgNetCDF<T>::loadFile(fileName,dimi_name,dimj_name,time_name);
    //then may use loadDimi(),loadDimj(),loadDimTime() to allocate
    return 0;
  }//loadFile

  //! print class dimensions
  void print(void)
  {
#if cimg_debug>10
    std::cerr << "CImgNetCDF_test::" << __func__ << "()" << std::endl;
#endif
    if(CImgNetCDF<T>::vpNCDim.size()>=1)std::cout<<"dimx="<<dimx()<<",";
    if(CImgNetCDF<T>::vpNCDim.size()>=2)std::cout<<"dimy="<<dimy()<<",";
    if(CImgNetCDF<T>::vpNCDim.size()>=3)std::cout<<"dimz="<<dimz()<<",";
    if(CImgNetCDF<T>::vpNCDim.size()==4)std::cout<<"dimv="<<dimv()<<",";
    std::cout<<"dimt="<<CImgNetCDF<T>::loadDimTime();
  }//print
};//CImgNetCDF_test





  //! add NetCDF read/write support to CImg class
  /**
   * an object of this class may be created or CImg may derived from this class to handle NetCDF
   * \li object in \c .cpp
   * \code
   *   CImgNetCDF<int> object;
   * \endcode
   * \li derivation in \c .h
   * \code
   *   template<typename T> CnewClass: \c CImg<T>, \c CImgNetCDF<T>
   * \endcode
   *
   * \note \c CImgNetCDF and \c CImg must be of the same type
   * \note For this implementation, NetCDF variable dimensions need to be 5: four fixed dimensions (x,y,z,v) and one unlimited dimension (t)
   *
   * \code
   *   
   * \endcode
   *
   * \see CImg
   *
   **/
  template<typename T> class CImgListNetCDF : public CImgNetCDF<T>
  {
    //variable
  public:
    //! pointers to NetCDF data
    /**
     * access to NetCDF data
     *
     * \code
     *   
     * \endcode
     *
     **/
    std::vector<NcVar*> pNCvars;
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
    int addNetCDFDims(CImgList<T> &imgs,std::vector<std::string> dim_names,std::string time_name)
    {
#if cimg_debug>10
      std::cerr << "CImgListNetCDF::" << __func__ << "(CImgList<" << imgs.pixel_type() << ">(" << imgs.size << "),";
      //  std::cerr << ((dim_names==NULL)?"default":dimi_name) << "," << ((dimj_name==NULL)?"default":dimj_name) << ",";
      std::cerr << time_name << ")" << std::endl;
#endif
      //check sizes
      if(imgs.size()<1) {return DIM_ERROR;}
      return CImgNetCDF<T>::addNetCDFDims(imgs(0),dim_names,time_name);
    }//addNetCDFDims
  //! \overload without unlimited direction
  int addNetCDFDims(CImgList<T> &img,std::vector<std::string> dim_names)
  {
    std::string time_name;
    return addNetCDFDims(img,dim_names,time_name);
  }//addNetCDFDims
    //! add variables in NetCDF file
    /**
     * save variables in NetCDF file, may be called after creation of dimensions \c CImgNetCDF::addNetCDFDims
     *
     * \code
     *   dimv=2;char *fileName="2D2C.nc";
     *   C2DnCNetCDFvelocity vel;//this class MUST be derivated as dimi(), dimj() and spectrum() are VIRTUAL
     *   vel.pNCFile=new NcFile(fileName,NcFile::Replace);
     *   vel.addNetCDFDims("dimX","dimY","time");
     *   std::vector<std::string> var_names(dimv+1);
     *   var_names[0]="vel_X";var_names[1]="vel_Y";
     *   var_names[2]="flag";
     *   std::vector<std::string> unit_names(2);
     *   unit_names[0]="m/s";unit_names[1]="none";
     *   addNetCDFVar(&var_names,&unit_names);
     * \endcodeif((*this).loadDim(0)!=imgs(0).width()) {return DIM_ERROR;}
     *
     * \see CImgNetCDF::addNetCDFDims
     *
     **/
    int addNetCDFVar(CImgList<T> &imgs,std::vector<std::string> var_names,std::vector<std::string> unit_names)
    {
#if cimg_debug>10
      std::cerr << "CImgListNetCDF::" << __func__ << "(CImgList<" << imgs.pixel_type() << ">(" << imgs.size << "),";
      // << var_names << "," << unit_names
      std::cerr << ")" << std::endl;
#endif
      //check sizes
      if(imgs.size()<1) {return DIM_ERROR;}
      if(var_names.size()!=unit_names.size()) {return DIM_ERROR;}
      //check dims
      for(unsigned int a=0 ; a<CImgNetCDF<T>::vpNCDim.size() ; a++){
	switch(a){
	case 0:if((*this).loadDim(a)!=imgs(0).width()) {return DIM_ERROR;}break;
	case 1:if((*this).loadDim(a)!=imgs(0).height()) {return DIM_ERROR;}break;
	case 2:if((*this).loadDim(a)!=imgs(0).depth()) {return DIM_ERROR;}break;
	case 3:if((*this).loadDim(a)!=imgs(0).spectrum()) {return DIM_ERROR;}break;
	}
      }
	
      //allocates
      pNCvars.assign(imgs.size(),NULL);
      //var component
      for(unsigned int i=0;i<pNCvars.size();i++)
	{
	  if((*this).CImgNetCDF<T>::addNetCDFVar(var_names[i],unit_names[i])) {return NC_ERROR;}
	  pNCvars[i]=(*this).pNCvar;(*this).pNCvar=NULL;
	}
      return 0;
    }//addNetCDFVar




    //! add variable data in NetCDF file
    /**
     * add variable data in NetCDF file, may be called after creation of both dimensions \c CImgNetCDF::addNetCDFDims and variables \c CImgNetCDF::addNetCDFVar 
     *
     * \code
     *   
     * \endcode
     *
     * \see CImgListNetCDF::addNetCDFDims CImgListNetCDF::addNetCDFVar
     *
     **/
    int addNetCDFData(CImgList<T> &imgs,int time=-1)
    {
#if cimg_debug>10
      std::cerr << "CImgListNetCDF::" << __func__ << "(CImgList<" << imgs.pixel_type() << ">(" << imgs.size << ")";
      std::cerr << ",time=" << time << " (if -1 append time))" << std::endl;
#endif
      if(pNCvars.empty()) return CODE_ERROR;
      //check sizes
      if(imgs.size()!=pNCvars.size()) {return DIM_ERROR;}
      //set time if needed (append mode)
      if(time==-1) time=(*this).CImgNetCDF<T>::loadDimTime();
      //var component
      for(unsigned int i=0;i<pNCvars.size();i++)
	{
	  //write data
	  (*this).pNCvar=pNCvars[i];//set current variable data pointer
	  if((*this).CImgNetCDF<T>::addNetCDFData(imgs[i],time)) {return NC_ERROR;}
	  (*this).pNCvar=NULL;
	}
      return 0;
    }//addNetCDFData






    //! load variable dimension from NetCDF file
    /**
     * load from NetCDF file a certain number of variables from names, that will set the variable dimension
     *
     * \code
     *   
     * \endcode
     *
     **/
    int loadNetCDFVar(CImgList<T> &imgs,std::vector<std::string> &var_names,std::vector<std::string> &unit_names)
    {
#if cimg_debug>10
      std::cerr << "CImgListNetCDF::" << __func__ << "(CImgList<" << imgs.pixel_type() << ">(" << imgs.size << ")";
      std::cerr << ",vector<string>,vector<string>)" << std::endl;
#endif
      //allocates
      pNCvars.assign(var_names.size(),NULL);
      //allocates if needed
      if(imgs.is_empty()) {
	switch(CImgNetCDF<T>::vpNCDim.size()){
	case 4:imgs.assign(pNCvars.size(),(*this).loadDim(0),(*this).loadDim(1),(*this).loadDim(2),(*this).loadDim(3));break;
	case 3:imgs.assign(pNCvars.size(),(*this).loadDim(0),(*this).loadDim(1),(*this).loadDim(2));break;
	case 2:imgs.assign(pNCvars.size(),(*this).loadDim(0),(*this).loadDim(1));break;
	case 1:imgs.assign(pNCvars.size(),(*this).loadDim(0));break;
	  //default:NULL;
	}
      }
      //var component
      for(unsigned int i=0;i<pNCvars.size();i++)
	{
	  //load variable and allocates if needed
	  if((*this).CImgNetCDF<T>::loadNetCDFVar(imgs[i],var_names[i],unit_names[i])) {return NC_ERROR;}//### pUnit_names
	  pNCvars[i]=(*this).pNCvar;//set current variable data pointer
	  (*this).pNCvar=NULL;
	}
      return 0;
    }//loadNetCDFVar





    //! load variable data from NetCDF file
    /**
     * load from NetCDF file variable data at a time
     *
     * \arg img: load data values in this \c CImg
     * \arg time: get data at time \c time (must be in the range 0 to < \c loadDimTime())
     *
     * \code
     *   
     * \endcode
     *
     **/
    int loadNetCDFData(CImgList<T> &imgs,int time=0)
    {
#if cimg_debug>10
      std::cerr << "CImgListNetCDF::" << __func__ << "(CImgList<" << imgs.pixel_type() << ">(" << imgs.size << ")";
      std::cerr << ",time=" << time << ")" << std::endl;
#endif
      if(pNCvars.empty()) return CODE_ERROR;
      //check sizes
      if(imgs.size()!=pNCvars.size()) {return DIM_ERROR;}
      //var component
      for(unsigned int i=0;i<pNCvars.size();i++)
	{
	  //write data
	  (*this).pNCvar=pNCvars[i];//set current variable data pointer
	  if((*this).CImgNetCDF<T>::loadNetCDFData(imgs[i],time)) {return NC_ERROR;}
	  (*this).pNCvar=NULL;
	}
      return 0;
    }//loadNetCDFData
    int loadNetCDFData3D(CImgList<T> &imgs,int time=0)
    {
#if cimg_debug>10
      std::cerr << "CImgListNetCDF::" << __func__ << "(CImgList<" << imgs.pixel_type() << ">(" << imgs.size << ")";
      std::cerr << ",time=" << time << ")" << std::endl;
#endif
      if(pNCvars.empty()) return CODE_ERROR;
      //check sizes
      if(imgs.size!=pNCvars.size()) {return DIM_ERROR;}
      //var component
      for(unsigned int i=0;i<pNCvars.size();i++)
	{
	  //write data
	  (*this).pNCvar=pNCvars[i];//set current variable data pointer
	  if((*this).CImgNetCDF<T>::loadNetCDFData3D(imgs[i],time)) {return NC_ERROR;}
	  (*this).pNCvar=NULL;
	}
      return 0;
    }//loadNetCDFData3D
    int loadNetCDFData2D(CImgList<T> &imgs,int time=0)
    {
#if cimg_debug>10
      std::cerr << "CImgListNetCDF::" << __func__ << "(CImgList<" << imgs.pixel_type() << ">(" << imgs.size << ")";
      std::cerr << ",time=" << time << ")" << std::endl;
#endif
      if(pNCvars.empty()) return CODE_ERROR;
      //check sizes
      if(imgs.size!=pNCvars.size()) {return DIM_ERROR;}
      //var component
      for(int i=0;i<pNCvars.size();i++)
	{
	  //write data
	  (*this).pNCvar=pNCvars[i];//set current variable data pointer
	  if((*this).CImgNetCDF<T>::loadNetCDFData2D(imgs[i],time)) {return NC_ERROR;}
	  (*this).pNCvar=NULL;
	}
      return 0;
    }//loadNetCDFData2D
  };//CImgListNetCDF

#endif/*CIMG_NETCDF*/

