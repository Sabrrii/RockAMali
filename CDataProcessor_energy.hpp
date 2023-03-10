#ifndef _DATA_PROCESSOR_ENERGY_
#define _DATA_PROCESSOR_ENERGY_

#include "CDataProcessor.hpp"

//! base class for energy processing (should not be used, just for inheritance)
template<typename Tdata, typename Tproc=Tdata, typename Taccess=unsigned char>
class CDataProcessor_energy : public CDataProcessor_kernel<Tdata,Tproc, Taccess>
{
public:
  CDataProcessor_energy(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessor_kernel<Tdata,Tproc, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check)
  {
    std::cout<<__FILE__<<"::"<<__func__<<"(...)"<<std::endl;
//    this->debug=true;
    this->class_name="CDataProcessor_energy";
    this->image.assign(1);//content: E only
    this->check_locks(lock);
  }//constructor

#ifdef DO_NETCDF
  virtual void set_var_unit_long_names(std::vector<std::string> &var_unit_long_names)
  {
    var_unit_long_names.push_back("E");
    var_unit_long_names.push_back("digit");
    var_unit_long_names.push_back("energy");
  }//set_var_unit_long_names
#endif //NetCDF

};//CDataProcessor_energy	

//! process a single simulated peak (look like PAC signal)
/**
 * find the max, min, threshold value and the position of the  trigger, maximum, threshold
 *
 * assign values to the image (signal, threshold, min while between max and treshold,  min while trigger)
 *
 * display the graph with 4 curves (signal, threshold, max and 36.8% height positions,  trigger and max)
 *
 * algorithm : 
 * - max : find the maximum value of the signal
 * - min : find the minimum value of the signal
 * - threshold : calculation of the 36.8% max value + baseline
 * 
 * \ref pageSchema "Signal schema" 
 *
**/
template<typename Tdata, typename Tproc=Tdata, typename Taccess=unsigned char>
class CDataProcessor_Max_Min : public CDataProcessor_energy<Tdata,Tproc, Taccess>
{
public:
  int A,B,Ai,Hi,Ti,threshold;

  CDataProcessor_Max_Min(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessor_energy<Tdata,Tproc, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check)
  {
    std::cout<<__FILE__<<"::"<<__func__<<"(...)"<<std::endl;
    this->debug=true;
    this->class_name="CDataProcessor_Max_Min";
    this->check_locks(lock);
  }//constructor
  //! find important paramaters and their position
  virtual void Process(CImg<Tdata> &in, int &ampli, int &base, int &Imax, int &Ith, int &Itrig, int &th) 
  {
	//find the min and max Amplitude
	base = in.min();
	ampli = in.max() - base;	
	for (int i=0;in(i)== base; i++)
	{//finding the trigger position
	  Itrig=i;
	}			
	for (int i=0; in(i)< ampli + base; i++)
	{// finding the position of the maximum Amplitude
	   Imax= i+1;
	} 		           	
	th = ampli*0.368 + base;
        //Index of the threshold start where the Index of the max end
	Ith=Imax; 
	while (in(Ith) > th) 
  	{// find the position of 36.8% amplitude
	   Ith++; 
	}
  }//Process_Data
  //! display the process signal
  #if cimg_display!=0
  virtual void Display(CImg<Tdata> &signal, int ampli, int base, int Imax, int Ith, int Itrig, int th) 
  {
    	CImg<Tproc> imageC;
	// make 4 curves with the numbers of items and fills it with 0
	imageC.assign(signal.width(),1,1,4,0);
        //change the specified channel to the paramaters values
	imageC.get_shared_channel(0)+=signal;
	imageC.get_shared_channel(1)+=th;
	imageC.get_shared_channel(2)+=ampli+base;
	imageC.get_shared_channel(3)+=ampli+base;
	//put x at baseline while amplitude is > threshold 
	cimg_for_inX(imageC,Imax,Ith,i) imageC(i,0,0,2)=base;
        //put x at baseline at the trigger  
	cimg_for_inX(imageC,Itrig,Imax,i) imageC(i,0,0,3)=base;
        imageC.display_graph("red = signal, green = threshold, blue = max and 36.8% height positions, yellow = trigger and max");
  }//Process_Data
  #endif //cimg_display
 
  //! compution kernel for an iteration
  virtual void kernelCPU_Max_Min(CImg<Tdata> &in,CImg<Tproc> &out)
  {  
    Process(in, A,B,Ai,Hi,Ti,threshold);
    #if cimg_display!=0
    Display(in, A,B,Ai,Hi,Ti,threshold);
    #endif
    out(0)=A;
  };//kernelCPU_Max_Min

  //! compution kernel for an iteration
  virtual void kernelCPU(CImg<Tdata> &in,CImg<Tproc> &out)
  {
    kernelCPU_Max_Min(in,out);
  };//kernelCPU
};//CDataProcessor_Max_Min	

#ifdef DO_NETCDF

//! trapezoidal data from CDL parameters
/**
 * get from CDL parameter values for trapezoidal filter (including simple dicri ones)
**/
template<typename Tdata, typename Tproc>
class CData_Filter
{
public:
  int k, m, n, q, Tm, decalage;
  float /*Tproc*/ threshold;
  float alpha, fraction;

  //! read processing parameters from CDL parameter file (as .nc)
  int read_parameters(int &ks, int &ms, int &number, int &qDelay, int &Tpeak, float &th, float &alp, float &frac)
  {
    ///file name
    std::string fi="parameters.nc";//=cimg_option("-p","parameters.nc","comment");
    double Alpha, Fraction, Threshold;
    ///parameter class
    CParameterNetCDF fp;
    //open file
    int error=fp.loadFile((char *)fi.c_str());
    if(error){std::cerr<<"loadFile return "<< error <<std::endl;return error;}

    float process; 
    std::string process_name="trapezoid";
    //load process variable
    error=fp.loadVar(process,&process_name);
    if(error){std::cerr<<"loadVar return "<< error <<std::endl;return error;}
    std::cout<<process_name<<"="<<process<<std::endl;
    ///k
    std::string attribute_name="k";
    if ((error = fp.loadAttribute(attribute_name,ks))!=0){
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<ks<<std::endl;
    ///m
    attribute_name="m";
    if ((error = fp.loadAttribute(attribute_name,ms))!=0){
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<ms<<std::endl;
    ///alpha
    attribute_name="alpha";
    if ((error = fp.loadAttribute(attribute_name,Alpha))!=0){
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<Alpha<<std::endl;
    ///n
    attribute_name="n";
    if ((error = fp.loadAttribute(attribute_name,number))!=0){
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<number<<std::endl;
    ///q
    attribute_name="q";
    if ((error = fp.loadAttribute(attribute_name,qDelay))!=0){
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<qDelay<<std::endl;
    ///threshold
    attribute_name="threshold";
    if ((error = fp.loadAttribute(attribute_name,Threshold))!=0){
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<Threshold<<std::endl;
    ///fraction
    attribute_name="fraction";
    if ((error = fp.loadAttribute(attribute_name,Fraction))!=0){
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
    return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<Fraction<<std::endl;
    ///Tm
    attribute_name="Tm";
    if ((error = fp.loadAttribute(attribute_name,Tpeak))!=0){
      std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
      return error;
    }
    std::cout<<"  "<<attribute_name<<"="<<Tpeak<<std::endl;

    alp=Alpha;
    frac=Fraction;
    th=Threshold;
    return 0;
  }//read_parameters

  //! read processing parameters from CDL parameter and setup other variable for processing
  int read_parameters_and_setup()
  {
    //read CDL
    read_parameters(k,m,n,q,Tm,threshold, alpha,fraction);
    //setup for processing
    decalage= 2*k+m+2;
  }//read_parameters_and_setup

  //! fill the image with the filter
  void trapezoidal_filter(const CImg<Tdata> &e, CImg<Tproc> &s, int ks, int ms, double alp, int decalage) 
  {
    //initiation of first parts of data
    cimg_for_inX(s,0,decalage-1,n) s(n)=e(0);
    //create a filter
    cimg_for_inX(s,decalage, s.width()-1,n)
    s(n)=2*s(n-1)-s(n-2) + e(n-1)-alp*e(n-2) -e(n-(ks+1)) \
			+alp*e(n-(ks+2))-e(n-(ks+ms+1))+alp*e(n-(ks+ms+2)) \
					+e(n-(2*ks+ms+1))-alp*e(n-(2*ks+ms+2));		
  }//trapezoidal_filter

  //! find position of the trigger
  int Calcul_Ti(const CImg<Tproc> &s, float th) 
  {
    //find the position of the trigger
    int Ti=0;
    for (int i=0;s(i) < th; i++)
    {
      Ti=i+1;
    }
    return Ti;
  }//Calcul_Ti
  
  //! calculation of the energy based on the formula (peak-base)/number
  Tproc Calculation_Energy(const CImg<Tproc> &trapezoid, int Ti,int number, double qDelay)
  {
    //sum of the n baseline value
    int base=0;
    cimg_for_inX(trapezoid,Ti-number, Ti,i) base+=trapezoid(i);
    //sum of the n peak value
    int peak=0;
    cimg_for_inX(trapezoid,Ti+qDelay, Ti+qDelay+number,i) peak+=trapezoid(i);
    //print both sum and return the energy 
    std::cout<<"base="<<base/number<<std::endl;
    std::cout<<"peak="<<peak/number<<std::endl;
    return (Tproc)(peak-base)/(Tproc)number;
  }//Calculation_Energy
//! \todo [medium] add normalisation of computed energy, so E simulated is close to E measured !
};//CData_Filter

//! process a single peak from PAC signal 
/**
 * Calculation of a trapezoidal based on the signal input
 * Display a graph with 3 curves (signal input, trapezoidal filter normalize and the max when the computation begin)
 *
 * Create a discri simple and a discri threshold, find and return the position of the trigger
 * Display a graph with 4 curves ( discri simple, dCFD, threshold and the signal)
 *
 * Display a graph with 4 curves (Filter, N baseline, Q delay, N flat top)
 * Make the sum of baseline and peak to calculate the energy ((peak-base)/n) then return it
 *
 * Parameters NetCDF CDL : 
 * - k= increase size
 * - m= plateau size
 * - B= Baseline
 * - n= N baseline
 * - q= Computing Delay
 * - Tm= peak time
 * - threshold= should be 36.8 % of the amplitude
 * - alpha= 
 * - fraction=
 *
 * \ref pageSchema "Signal schema" 
 *
**/
template<typename Tdata, typename Tproc=Tdata, typename Taccess=unsigned char>
class CDataProcessor_trapezoid : public CDataProcessor_energy<Tdata,Tproc, Taccess>, public CData_Filter<Tdata,Tproc>
{
public:
  bool image_assigned;
  CImg<Tproc> s,imageDCF, trapezoid;

  CDataProcessor_trapezoid(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessor_energy<Tdata,Tproc, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check)
  {
    std::cout<<__FILE__<<"::"<<__func__<<"(...)"<<std::endl;
    this->debug=true;
    this->class_name="CDataProcessor_trapezoid";
    this->read_parameters_and_setup();
    image_assigned=false;
    this->check_locks(lock);
  }//constructor

  //! display the signal, the filter and the computation start
  #if cimg_display!=0
  virtual void Display(CImg<Tdata> in, CImg<Tproc> out, int decalage)
  {
	CImg<Tproc> imageC;
	imageC.assign(in.width(),1,1,3,0);
	imageC.get_shared_channel(0)+=in;
	imageC.get_shared_channel(1)+=out*((Tproc)in.max()/out.max()); // trapezoid normalize
	cimg_for_inX(imageC,decalage,imageC.width(),i) imageC(i,0,0,2)=in.max();//begin of the trapezoid computation
	imageC.display_graph("red = signal, green = filter, blue = trapezoidal computation");
  }//Display
  #endif // #if cimg_display

  //!fill the image with 2 discri and display it, return the position of the trigger
  virtual void Calcul_Discri(const CImg<Tdata> &e, CImg<Tproc> &s,CImg<Tproc> &imageDCF,int Tpeak,double frac,double alp) 
  {
    if(!image_assigned) {s.assign(e.width());/*image_assigned=true;*/}
	int delay = (3*Tpeak)/2;
	//Discri simple
	s(0)=0;
	cimg_for_inX(s,1,s.width(),n) s(n)=e(n)-alp*e(n-1);
	//Discri treshold		
	if(!image_assigned) {imageDCF.assign(s.width(),1,1,1, 0);image_assigned=true;}
	cimg_for_inX(imageDCF,delay,s.width(),n) imageDCF(n)=s(n-delay)-frac*s(n);
  }//Calcul_Discri

  #if cimg_display!=0
  //!display 2 discri
  virtual int Discri_Display(CImg<Tdata> e, CImg<Tproc> s,CImg<Tproc> imageDCF,int Tpeak,Tproc th, int Ti, double frac,double alp) 
  {
	CImg<Tproc> imageC;
	imageC.assign(s.width(),1,1,5, 0);
	imageC.get_shared_channel(0)+=s;
	imageC.get_shared_channel(1)+=imageDCF;
	imageC.get_shared_channel(2)+=th;
	//yellow signal normalized
	imageC.get_shared_channel(3)+=e*(s.max()/(Tproc)e.max());
	//purple show where is the trigger
	cimg_for_inX(imageC,Ti,imageC.width(),i) imageC(i,0,0,4)=imageDCF.max();		
	imageC.display_graph("red = discri simple, green = dCFD, blue = threshold, yellow = signal, purple = trigger");	
  }//Discri_Display
  #endif //#cimg_display

  #if cimg_display!=0
  //! display the filter in details (signal, baseline, delay and flat top)
  void Display_Trapeze_Paramaters(CImg<Tproc> in, int Ti,int number, double qDelay)
  {
	CImg<Tproc> imageC;
	imageC.assign(in.width(),1,1,4,0);
	imageC.get_shared_channel(0)+=in;
	cimg_for_inX(imageC,Ti-number,Ti,i) imageC(i,0,0,1)=in.max();
	cimg_for_inX(imageC,Ti,Ti+qDelay,i) imageC(i,0,0,2)=in.max();
	cimg_for_inX(imageC,Ti+qDelay,Ti+qDelay+number,i) imageC(i,0,0,3)=in.max();
	imageC.display_graph("red = Filter, green = N baseline, Blue = Q delay, yellow = N flat top");
  }//Display_Trapeze_Paramaters
  #endif //#cimg_display

  //! compution kernel for an iteration
  virtual void kernelCPU_Trapeze(CImg<Tdata> &in,CImg<Tproc> &out)
  {    
    if(!image_assigned) {trapezoid.assign(in.width());/*image_assigned=true;*/}
    this->trapezoidal_filter(in,trapezoid, this->k,this->m,this->alpha, this->decalage);
    #if cimg_display!=0
    Display(in, trapezoid, this->decalage);
    #endif //#cimg_display   
    ///Discri   
    Calcul_Discri(in,s,imageDCF, this->Tm, this->fraction,this->alpha);
    int Ti=this->Calcul_Ti(s,this->threshold);
    std::cout<< "Trigger start= " << Ti  <<std::endl;
    #if cimg_display!=0
    Discri_Display(in,s,imageDCF, this->Tm,this->threshold,Ti, this->n, this->q);
    #endif //#cimg_display
    ///Energy
    float E=this->Calculation_Energy(trapezoid, Ti, this->n, this->q);
    std::cout<< "Energy= " << E  <<std::endl;
    #if cimg_display!=0
    ///trapezoidal
    Display_Trapeze_Paramaters(trapezoid, Ti, this->n, this->q);
    #endif //#cimg_display
    out(0)=E;
  };//kernelCPU_Trapeze

  //! compution kernel for an iteration
  virtual void kernelCPU(CImg<Tdata> &in,CImg<Tproc> &out)
  {
    kernelCPU_Trapeze(in,out);
  };//kernelCPU
};//CDataProcessor_trapezoid
#endif //NetCDF

#endif //_DATA_PROCESSOR_ENERGY_

