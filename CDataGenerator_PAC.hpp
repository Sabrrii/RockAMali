#ifndef _DATA_GENERATOR_PAC_
#define _DATA_GENERATOR_PAC_

#include "CDataGenerator.hpp"

#ifdef DO_NETCDF

//! generate a single peak close to PAC signal
/**
 * generate a single curve looking like PAC signal
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
 * 
 * \ref pageSchema "Signal schema" 
**/

template<typename Tdata, typename Taccess=unsigned char>
class CDataGenerator_Peak: public CDataGenerator<Tdata, Taccess>
{

public:
  int nb_tB,nb_tA,A,B;
  double tau; 
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
  }//iteration

};//CDataGenerator_Peak

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

  CDataGenerator_Peak_Noise(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_FILLED
  )
  : CDataGenerator_Peak<Tdata, Taccess>(lock,wait_status,set_status)
  {
//    this->debug=true;
    this->class_name="CDataGenerator_Peak_Noise";	
    Get_Parameters_Noise(rand_min,rand_max);	
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
  }//iteration

};//CDataGenerator_Peak_Noise


//! Explanation of signal paramaters
/**
  \page pageSchema Schema du signal
 * 
 * \image html Signal_details.png "PAC signal details"
 *
 *  graphic legend :  
 * - B: Baseline
 * - A: Amplitude
 * - nb_tA: peak duration
 * - nb_tB: baseline duration
 * - Tau: decrease time
 * - A * exp(-t/tau)+B: Exponential decrease 
 * - nbitem: size of image (x) 
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

#endif //_DATA_GENERATOR_PAC_

