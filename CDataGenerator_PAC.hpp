#ifndef _DATA_GENERATOR_PAC_
#define _DATA_GENERATOR_PAC_

#include "CDataGenerator.hpp"

#ifdef DO_NETCDF
#include "CImg_NetCDF.h"

//! generate a single peak close to PAC signal
/**
 * generate a single curve looking like PAC signal
 *
 * generate Peak data into a shared circular buffer
 * \note Peak data except first one that is frame count value
 *
 * * user input parameters are loaded from NetCDF CDL - \see struct_parameter_NetCDF.h -, e.g. parameters.cdl :
 * - B: base line
 * - A: Amplitude
 * - nb_tA: peak duration
 * - nb_tB: baseline duration
 * - Tau: decrease time
 * 
 * * computer output parameters might be stored in NetCDF - \see CImg_NetCDF.h - (e.g. pac_signal_parameters.nc) \see
 * - \c ncInit
 * - \c ncStore
 * 
 * \ref pageSchema "Signal schema" 
**/

template<typename Tdata, typename Taccess=unsigned char
#ifdef DO_NETCDF
, typename Tnetcdf=int
#endif //DO_NETCDF
>
class CDataGenerator_Peak: public CDataGenerator<Tdata, Taccess>
{

public:
  int nb_tB,nb_tA,A,B;
  double tau; 
#ifdef DO_NETCDF
  std::string file_name;
  CImgListNetCDF<Tnetcdf> nc;
  CImgList<Tnetcdf> nc_img;//temporary image for type conversion
  bool is_netcdf_init;
  //dimension names
  std::vector<std::string> dim_names;
  std::string dim_time;
  //variable names (and its unit)
  std::vector<std::string> var_names;
  std::vector<std::string> unit_names;
  std::vector<std::string> long_names;
#endif //DO_NETCDF
  //! get all parameters from NC file (i.e. compiled CDL)
  int Get_Parameters(int &nb_base, int &nb_peak, double &decrease, int &ampl, int &base)
  {
    int Tau;
     ///file name
     std::string fi="parameters.nc";//=cimg_option("-p","parameters.nc","comment");
  
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
     ///nb_tB
     std::string attribute_name="nb_tB";	// 10us
     if (error = fp.loadAttribute(attribute_name,nb_base)!=0)
     {
       std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
       return error;
     }
    std::cout<<"  "<<attribute_name<<"="<<nb_base<<std::endl;
    ///nb_tA
    attribute_name="nb_tA";		//100 ns
    if (error = fp.loadAttribute(attribute_name,nb_peak)!=0){
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<nb_peak<<std::endl;
    ///tau
    attribute_name="tau";			//5 us
    if (error = fp.loadAttribute(attribute_name,Tau)!=0){
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<Tau<<std::endl;
    ///A
    attribute_name="A";
    if (error = fp.loadAttribute(attribute_name,ampl)!=0){
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<ampl<<std::endl;		 
    ///B
    attribute_name="B";
    if (error = fp.loadAttribute(attribute_name,base)!=0){
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<base<<std::endl; 
    decrease=Tau; // convert into int
    return 0;

  }//Get_Parameters

  void Peak (CImgList<Tdata> &images, int n)//, int index) //fill image with Peak 
  {
	//Baseline
	cimg_for_inX(images[n],0,nb_tB,i) images[n](i)=B;
        //Peak
	const float step =(float)A/(nb_tA - nb_tB);
        int j=0;
        cimg_for_inX(images[n],nb_tB,nb_tA,i) images[n](i)=step*j++ +B;
	//Exponential decrease
	int t=0;
        cimg_for_inX(images[n],nb_tA,images[n].width(),i) images[n](i)=A * exp(-t++/tau)+B;  	
  }//Peak

#ifdef DO_NETCDF
  //! NetCDF initialisation
  virtual void ncInit()
  {
    file_name="pac_signal_parameters.nc";
    nc_img.assign(5, 1,1,1,1, -99);// A,B, tau, tA,tB
    std::cout << "CImgListNetCDF::saveNetCDFFile(" << file_name << ",...) return " << nc.saveNetCDFFile((char*)file_name.c_str()) << std::endl;
    is_netcdf_init=false;
    dim_time="dimF";
    dim_names.push_back("dim1");
    std::cout << "CImgListNetCDF::addNetCDFDims(" << file_name << ",...) return " << nc.addNetCDFDims(nc_img,dim_names,dim_time) << std::endl<<std::flush;
    //variable names (and its unit)
    var_names.push_back("A");
    var_names.push_back("B");
    var_names.push_back("tau");
    var_names.push_back("ta");
    var_names.push_back("tb");
    unit_names.push_back("digit");
    unit_names.push_back("digit");
    unit_names.push_back("tic (10ns)");
    unit_names.push_back("tic (10ns)");
    unit_names.push_back("tic (10ns)");
    long_names.push_back("amplitude");
    long_names.push_back("baseline");
    long_names.push_back("exponential decrease");
    long_names.push_back("baseline duration");
    long_names.push_back("increase duration");
    //variables
std::cout << "CImgListNetCDF::addNetCDFVar(" << file_name << ",...) return " << nc.addNetCDFVar(nc_img,var_names,unit_names) << std::endl<<std::flush;
    //variable long names
    cimglist_for(nc_img,x)
    {
      this->nc.pNCvars[x]->add_att("long_name",long_names[x].c_str());
      if (!(this->nc.pNCvars[x]->add_att("generator_",this->class_name.c_str()))) std::cerr<<"error: for PAC signal parameter in NetCDF, while adding generator name attribute"<<this->class_name<<" (NC_ERROR)."<<std::endl;
    }
  }//ncInit

  //! NetCDF storage
  virtual void ncStore()
  {
    int n=0;
    nc_img[n++]=A;
    nc_img[n++]=B;
    nc_img[n++]=tau;
    nc_img[n++]=nb_tA;
    nc_img[n++]=nb_tB;
std::cout << "CImgListNetCDF::addNetCDFData(" << file_name << ",...) return " << nc.addNetCDFData(nc_img) << std::endl;
    }//ncStore
#endif //DO_NETCDF

  //! constructor
  CDataGenerator_Peak(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_FILLED
  )
  : CDataGenerator<Tdata, Taccess>(lock,wait_status,set_status)
  {
//    this->debug=true;
    this->class_name="CDataGenerator_Peak";
    Get_Parameters(nb_tB, nb_tA, tau, A, B);//Signal Parameters	
    nb_tA+=nb_tB; //nb_tA is position
    this->check_locks(lock);
#ifdef DO_NETCDF
    ncInit();
#endif //DO_NETCDF
  }//constructor

  //! one iteration for any loop
  /**
   * entirely filled with the frame count value
  **/
  virtual void iteration(CImg<Taccess> &access,CImgList<Tdata> &images, int n, int index)
  {
    if(this->debug)
    {
      this->lprint.print("",false);
      printf("4 B%02d #%04d: ",n,index);fflush(stdout);
      access.print("access",false);fflush(stderr);
      this->lprint.unset_lock();
    }
    //wait lock
    unsigned int c=0;
    this->laccess.wait_for_status(access[n],this->wait_status,this->STATE_FILLING, c);//free,filling

     Peak (images, n);
    //set frame count value as first array value
//    images[n](0)=i;
//    images[n](images[n].width()-1)=i;

    //set filled
    this->laccess.set_status(access[n],this->STATE_FILLING,this->set_status, this->class_name[5],index,n,c);//filling,filled
#ifdef DO_NETCDF
    ncStore();
#endif //DO_NETCDF
  }//iteration

};//CDataGenerator_Peak


//! generate a single peak close to PAC signal with noise
/**
 * generate a single curve looking like PAC signal with some noise
 *
 * generate Peak data into a shared circular buffer
 * \note Peak data except first one that is frame count value
 *
 * parameters NetCDF CDL :
 * - B: base line
 * - A: Amplitude
 * - nb_tA: peak duration
 * - nb_tB: baseline duration
 * - Tau: decrease time
 * - noise: value of the noise added
 *
 * \ref pageSchema "Signal schema" 
**/

template<typename Tdata, typename Taccess=unsigned char>
class CDataGenerator_Peak_Noise: public CDataGenerator_Peak<Tdata, Taccess>
{
public:
  float rand_min,rand_max;
  CImg<float> Random;
  int Get_Parameters_Noise(float &min_noise, float &max_noise)
  {
    ///file name
    std::string fi="parameters.nc";//=cimg_option("-p","parameters.nc","comment");
    double rnd; 
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
   
    ///noise
    std::string attribute_name="noise";
    if (error = fp.loadAttribute(attribute_name,rnd)!=0){
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<rnd<<std::endl; 
  
    min_noise=-(rnd/2); // convert into int
    max_noise=rnd/2; // convert into int
    return 0;
  }//Get_Parameters_Noise

#ifdef DO_NETCDF
  //! NetCDF initialisation
  virtual void ncInit()
  {
    (this->nc).pNCFile->add_att("signal_noise_min",rand_min);
    (this->nc).pNCFile->add_att("signal_noise_max",rand_max);
    cimglist_for(this->nc_img,x)if (!(this->nc.pNCvars[x]->add_att("generator",this->class_name.c_str()))) std::cerr<<"error: for PAC signal parameter in NetCDF, while adding generator name attribute"<<this->class_name<<" (NC_ERROR)."<<std::endl;
    }//ncStore
#endif //DO_NETCDF

  CDataGenerator_Peak_Noise(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_FILLED
  )
  : CDataGenerator_Peak<Tdata, Taccess>(lock,wait_status,set_status)
  {
//    this->debug=true;
    this->class_name="CDataGenerator_Peak_Noise";	
    Get_Parameters_Noise(rand_min,rand_max);
#ifdef DO_NETCDF
    this->ncInit();
#endif //DO_NETCDF
    this->check_locks(lock);
  }//constructor

  //! one iteration for any loop
  /**
   * entirely filled with the frame count value
  **/
  virtual void iteration(CImg<Taccess> &access,CImgList<Tdata> &images, int n, int index)
  {
    if(this->debug)
    {
      this->lprint.print("",false);
      printf("4 B%02d #%04d: ",n,index);fflush(stdout);
      access.print("access",false);fflush(stderr);
      this->lprint.unset_lock();
    }

    //image random
    if(index == 0)Random.assign(images[n].width());
    Random.rand(rand_min,rand_max);
    //wait lock
    unsigned int c=0;
    this->laccess.wait_for_status(access[n],this->wait_status,this->STATE_FILLING, c);//free,filling

    this->Peak (images, n);
    //add noise on peak
    cimg_forX(images[n],i) images[n](i)+=Random(i);

    //set filled
    this->laccess.set_status(access[n],this->STATE_FILLING,this->set_status, this->class_name[5],index,n,c);//filling,filled
#ifdef DO_NETCDF
    this->ncStore();
#endif //DO_NETCDF

  }//iteration

};//CDataGenerator_Peak_Noise

//! generate a single peak close to PAC signal with differents values
/**
 * generate a single curve looking like PAC signal with a random values at each iteration
 *
 * generate Peak data into a shared circular buffer
 * \note Peak data except first one that is frame count value
 *
 * parameters NetCDF CDL :
 * - min_Amp: minimum Amplitude
 * - max_Amp: maximum Amplitude
 * - min_tA: minimum peak duration
 * - max_tA: maximum peak duration
 * - min_tB: minimum baseline duration
 * - max_tB: maximum baseline duration
 * - min_tau: minimum decrease time
 * - max_tau: maximum decrease time
 * 
 * \ref pageSchema "Signal schema" 
**/

template<typename Tdata, typename Taccess=unsigned char>
class CDataGenerator_Full_Random: public CDataGenerator_Peak_Noise<Tdata, Taccess>
{
public:
  Tdata min_Amp,max_Amp, min_tau,max_tau, min_tB,max_tB, min_tA, max_tA;
  
  int Read_Paramaters (Tdata &min_A, Tdata &max_A, Tdata &min_T, Tdata &max_T, Tdata &min_tb, Tdata &max_tb, Tdata &min_ta, Tdata &max_ta)
  {
    ///file name
    std::string fi="parameters.nc";//=cimg_option("-p","parameters.nc","comment");
    int min_Amplitude,max_Amplitude,min_Tau,max_Tau,min_baseline,max_baseline,min_tcroi,max_tcroi;
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
    ///min_Amp
    std::string attribute_name="min_Amp";
    if (error = fp.loadAttribute(attribute_name,min_Amplitude)!=0)
    {
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<min_Amplitude<<std::endl;
    ///max_Amp
    attribute_name="max_Amp";
    if (error = fp.loadAttribute(attribute_name,max_Amplitude)!=0)
    {
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<max_Amplitude<<std::endl;
    ///min_tau
    attribute_name="min_tau";
    if (error = fp.loadAttribute(attribute_name,min_Tau)!=0)
    {
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<min_Tau<<std::endl;
    ///max_tau
    attribute_name="max_tau";
    if (error = fp.loadAttribute(attribute_name,max_Tau)!=0)
    {
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<max_Tau<<std::endl;
    ///min_tB
    attribute_name="min_tB";
    if (error = fp.loadAttribute(attribute_name,min_baseline)!=0)
    {
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<min_baseline<<std::endl;
    ///max_tB
    attribute_name="max_tB";
    if (error = fp.loadAttribute(attribute_name,max_baseline)!=0)
    {
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<max_baseline<<std::endl;
     ///min_tA
    attribute_name="min_tA";
    if (error = fp.loadAttribute(attribute_name,min_tcroi)!=0)
    {
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<min_tcroi<<std::endl;
    ///max_tA
    attribute_name="max_tA";
    if (error = fp.loadAttribute(attribute_name,max_tcroi)!=0)
    {
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<max_tcroi<<std::endl;  

    min_A=min_Amplitude; // convert int into Tdata
    max_A=max_Amplitude; // convert into int
    min_T=min_Tau; // convert into int
    max_T=max_Tau; // convert into int
    min_tb=min_baseline; // convert into int
    max_tb=max_baseline; // convert into int
    min_ta=min_tcroi; // convert into int
    max_ta=max_tcroi; // convert into int
  } //Read_Paramaters

  CDataGenerator_Full_Random(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_FILLED
  )
  : CDataGenerator_Peak_Noise<Tdata, Taccess>(lock,wait_status,set_status)
  {
//    this->debug=true;
    this->class_name="CDataGenerator_Full_Random";
    this->check_locks(lock);
    Read_Paramaters(min_Amp,max_Amp, min_tau,max_tau, min_tB,max_tB, min_tA, max_tA);
    cimglist_for(this->nc_img,x)if (!(this->nc.pNCvars[x]->add_att("generator",this->class_name.c_str()))) std::cerr<<"error: for PAC signal parameter in NetCDF, while adding generator name attribute"<<this->class_name<<" (NC_ERROR)."<<std::endl;

 }//constructor

  //! one iteration for any loop
  /**
   * entirely filled with the frame count value
  **/
  virtual void iteration(CImg<Taccess> &access,CImgList<Tdata> &images, int n, int index)
  {
    if(this->debug)
    {
      this->lprint.print("",false);
      printf("4 B%02d #%04d: ",n,index);fflush(stdout);
      access.print("access",false);fflush(stderr);
      this->lprint.unset_lock();
    }
    //wait lock

    //random values for curve parameters
    this->tau =  rand()%(max_tau-min_tau+1)+min_tau;
    std::cout<<"tau = "<<this->tau<<std::endl; 
    this->A =  rand()%(max_Amp-min_Amp+1)+min_Amp; 
    std::cout<<"Amplitude = "<<this->A<<std::endl; 
    this->nb_tB =  rand()%(max_tB-min_tB+1)+min_tB;
    std::cout<<"nb_tB = "<<this->nb_tB<<std::endl; 
    this->nb_tA =  rand()%(max_tA-min_tA+1)+min_tA; 
    std::cout<<"nb_tA = "<<this->nb_tA<<std::endl; 
    this->nb_tA+=this->nb_tB;
    std::cout<<"nb_tA+nb_tB = "<<this->nb_tA<<std::endl;
#ifdef DO_NETCDF
    if(!(this->is_netcdf_init))
    {
      //add class name in NetCDF profiling file
     cimglist_for (this->nc_img,x)if (!(this->nc.pNCvars[x]->add_att("generator",this->class_name.c_str()))) std::cerr<<"error: for PAC signal parameter in NetCDF, while adding generator name attribute"<<this->class_name<<" (NC_ERROR)."<<std::endl;
      this->is_netcdf_init=true;
    }//!is_netcdf_init
    //add data to NetCDF profiling file
    this->ncStore();
#endif //DO_NETCDF
     //noise
    if(index ==0)this->Random.assign(images[n].width());
    this->Random.rand(this->rand_min,this->rand_max);

    unsigned int c=0;
    this->laccess.wait_for_status(access[n],this->wait_status,this->STATE_FILLING, c);//free,filling

    //create image with the random paramaters
    this->Peak(images, n); 

    //add noise on peak
    images[n]+=this->Random;

    //set filled
    this->laccess.set_status(access[n],this->STATE_FILLING,this->set_status, this->class_name[5],index,n,c);//filling,filled
  }//iteration

};//CDataGenerator_Full_Random


#endif //NetCDF

/**
  \page pageSchema PAC Signal
 *
 * Generation of PAC signal and its processing are described here
 *
 * \li \ref sectionDisplay
 * \li \ref sectionGenerator
 * \li \ref sectionProcessor
 *
 *
 ** \section sectionDisplay Compile & execute the program
 * \image html Makefile_doc.png "Line to change & commands to type"
 *
 * - Makefile : \n
 *  \c GEN_FCT = name of the signal to be generated (in our case "signal_pac") \n
 *  \c PROC = name of the processor (in our case "filter") \n
 *  see FACTORIES for full available names :
   \code
     cat FACTORIES
     #or
     ./process_sequential -h 2>&1 | grep '\-factory\-'
   \endcode
 * e.g.
 * \verbinclude "FACTORIES"
 * - Terminal : \n
 *  \c make \c process_sequentialX = create the executable that will generate, process and store the signal \n
 *  \c make \c process-sequentialX_run = launch the executable who prints and displays the results in Xwindows (using CImg) \n
 *  \c make \c process_sequentialX_run, e.g.
   \code
    #generate compiled parameter file
    ncgen parameters.cdl -o parameters.nc && rm sample_sequential.nc

    #run program
    ./process_sequential.X -s 4096 -n 12 \
    -o sample_sequential.nc -r result_sequential.nc \
    --generator-factory signal_pac \
    --CPU-factory filter \
    --use-GPU --GPU-factory discri \
    --do-check --show

    #show result
    ncdump -h sample_sequential.nc \n
   \endcode
 * \note ncgen allow to get the user paramaters from the .cdl file, e.g. \c parameters.cdl
 * 
 ** \section sectionGenerator Generation of PAC signal
 * \image html Signal_details.png "PAC signal details"
 *
 *   Graphic legend :   
 * - \c blue curve : signal pac values (y axis)
 * - \c B: Baseline					(20)
 * - \c A: Amplitude					(1234)
 * - \c nb_tA: peak duration				(10)
 * - \c nb_tB: baseline duration				(1000)
 * - \c Tau: decrease time					(500)
 * , i.e. A * exp(-t/tau)+B: Exponential decrease 
 * - \c nbitem: size of frame (x axis) 				(4096)
 *
 ** \section sectionProcessor PAC signal processing
 * \image html filter_details.png "Trapezoidal filter details"
 *
 *  Graphic legend :  
 * - \c Signal: Signal Pac representation	
 * - \c Filter: show the energy with the formula : \n
 *  <I> s(n)=2*s(n-1)-s(n-2) + e(n-1)-alp*e(n-2) -e(n-(ks+1)) +alp*e(n-(ks+2))-e(n-(ks+ms+1))+alp*e(n-(ks+ms+2))+e(n-(2*ks+ms+1))-alp*e(n-(2*ks+ms+2)) </I> \n
 *  where alp=alpha ; s= trapezoidal; e=signal pac ; ks = increase size ; ms = plateau size;
 *  \note Filter Computation: Represent the part where computation of the filter is done
 * 
 * \image html discri_details.png "discri details"
 *
 *  Graphic legend :  
 * - \c DCFD: (n-delay)-frac*s(n)
 * - \c Discri \c simple: e(n)-alp*e(n-1)
 * - \c Threshold : value who serve as reference
 * - \c Signal : Signal Pac representation
 * - \c Trigger : end of the baseline, this is the result of the computation \n
 *
 * \image html ParaFilter_details.png "Paramaters filter trapezoidal details"
 *
 *  Graphic legend :  
 * - \c N \c baseline : n values of baseline
 * - \c Q \c delay : Time between the trigger and the max
 * - \c N \c flat \c top : n values at max
 * - \c Filter : trapezoidal filter
**/

#endif //_DATA_GENERATOR_PAC_

