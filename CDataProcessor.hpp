#ifndef _DATA_PROCESSOR_
#define _DATA_PROCESSOR_


//Package CImg
#include <CImg.h>

using namespace cimg_library;

#include "CDataBuffer.hpp"

#ifdef DO_NETCDF
#include <netcdfcpp.h>
#include "struct_parameter_NetCDF.h"
#endif

template<typename Tdata, typename Tproc, typename Taccess=unsigned char>
class CDataProcessor : public CDataBuffer<Tdata, Taccess>
{
public:
  //Processing buffer
  CImg<Tproc> image;
  //! result access
  CAccessOMPLock laccessR;
  CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR;
  CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR;

  bool do_check;unsigned int check_error;

  CDataProcessor(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataBuffer<Tdata, Taccess>(lock,wait_status,set_status)
  , laccessR(lock[2])
  , wait_statusR(wait_statusR), set_statusR(set_statusR)
  , do_check(do_check)
  {
//    this->debug=true;
    this->class_name="CDataProcessor";

    //this->check_locks(lock);
    if(lock.size()<3)
    {
      printf("code error: locks should have at least 3 locks for %s class.",this->class_name.c_str());
      exit(99);
    }
    check_error=0;
  }//constructor
  //! run for loop
  virtual void run(CImg<Taccess> &access,CImgList<Tdata> &images,  CImg<Taccess> &accessR,CImgList<Tproc> &results, unsigned int count, unsigned int stride=1, unsigned int start=0)
  {
    unsigned int nbuffer=images.size();
    for(unsigned int n=start,i=start;i<count;i+=stride)
    {
      this->iteration(access,images, accessR,results, n,i);
      //circular buffer
//! \bug needed: nbuffer should be a multiple of process thread number
      n+=stride;
      if(n>nbuffer-1) n=start;
     }//vector loop
  }//run

  //! compution kernel for an iteration
  virtual void kernel(CImg<Tdata> &in,CImg<Tproc> &out)
  {
    std::cout<< __FILE__<<"/"<<__func__<<"(images: in="<<in.width()<<", out="<<out.size()<<") copy kernel, other kernels should be implemented in inherited class."<<std::endl<<std::flush;
    out=in;
  };//kernel

  //! check data operation (test only, as very long compution function, and might lock data access for other threads)
  /**
   * check local result data (i.e. \c image member of this class) regarding to processed input data (i.e. \c img e.g. \c images[n]).
   * processing of input data to result one is on CPU and takes lot of time regarding to normal processing: test only !
   * \warning input data should be locked during this function, e.g. data not available for other threads
  **/
  virtual bool check_data(CImg<Tdata> &img, int i)
  {
    if(do_check)
    {
//std::cout<<__FILE__<<"::"<<__func__<<"(...)"<<std::endl;
//      return (image==img);
      return (image==i);
    }//do_check
    return true;
  }//check_data

  //! one iteration for any loop
  virtual void iteration(CImg<Taccess> &access,CImgList<Tdata> &images, CImg<Taccess> &accessR,CImgList<Tproc> &results, int n, int i)
  {
    if(this->debug)
    {
      this->lprint.print("",false);
      printf("4 B%02d #%04d: ",n,i);fflush(stdout);
      access.print("access",false);fflush(stderr);
      this->lprint.unset_lock();
    }

    //! 1. compute from buffer
    //wait lock
    unsigned int c=0;
    this->laccess.wait_for_status(access[n],this->wait_status,this->STATE_PROCESSING, c);//filled, processing
    //compution in local
    kernel(images[n],image);

    //check
    if(do_check)
    {
//std::cout<<__FILE__<<"::"<<__func__<<"(...) do check data"<<std::endl;
      //check input
      //if(images[n]==i) //slow check, but entire frame
      if(images[n](0)==i) //fast random check
      NULL; else {++check_error;std::cout<<"compution error: bad generate class for this test."<<std::endl<<std::flush;}
      //check output
      if(!check_data(images[n],i)) {++check_error;std::cout<<"compution error: bad check (i.e. test failed) on iteration #"<<i<<" (value="<<image(0)<<")."<<std::endl<<std::flush;}
    }//do_check

    //unlock
    this->laccess.set_status(access[n],this->STATE_PROCESSING,this->set_status, this->class_name[5],i,n,c);//processing, processed

    //! 2. copy to buffer
    if(this->debug)
    {
      this->lprint.print("",false);
      printf("4 C%02d #%04d: ",n,i);fflush(stdout);
      accessR.print("accessR",false);fflush(stderr);
      this->lprint.unset_lock();
    }

    //wait lock
    c=0;
    this->laccessR.wait_for_status(accessR[n],this->wait_statusR,this->STATE_PROCESSING, c);//filled, processing
    //copy local to buffer
    results[n]=image;
    //unlock
    this->laccessR.set_status(accessR[n],this->STATE_PROCESSING,this->set_statusR, this->class_name[5],i,n,c);//processing, processed

  }//iteration

  virtual void show_checking()
  {
    if(do_check)
    {
    if(check_error>0) std::cout<<"test: fail ("<<check_error<<" errors)";
    else std::cout<<"test: pass";
    std::cout<<" (for "<<this->class_name<<")."<<std::endl;
    }//do_check
  }//show_checking

};//CDataProcessor

//! complex operation for CPU process (intended for GPU lambda)
/**
 * val+val*val
**/
template<typename Tdata, typename Tproc>
void kernelCPU_vPvMv(CImg<Tdata> &in,CImg<Tproc> &out)
{
  out=in;
  cimg_forX(in,x) out(x)+=in(x)*in(x);
}//kernelCPU_vPvMv
//! complex operation for CPU process (intended for GPU lambda)
/**
 * val+val*val
**/
template<typename Tdata, typename Tproc=Tdata, typename Taccess=unsigned char>
class CDataProcessor_vPvMv : public CDataProcessor<Tdata,Tproc, Taccess>
{
public:
  CDataProcessor_vPvMv(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessor<Tdata,Tproc, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check)
  {
    this->debug=true;
    this->class_name="CDataProcessor_vPvMv";
std::cout<<__FILE__<<"::"<<__func__<<"(...)"<<std::endl;
    this->check_locks(lock);
  }//constructor

  virtual bool check_data(CImg<Tdata> &img, int i)
  {
//std::cout<<__FILE__<<"::"<<__func__<<"/"<<this->class_name<<"(...)"<<std::endl;
    if(this->do_check)
    {
      CImg<Tproc> imgt(img);
      kernelCPU(img,imgt);
      return (this->image==imgt);
    }//do_check
    return true;
  }//check_data

  //! compution kernel for an iteration
  virtual void kernelCPU(CImg<Tdata> &in,CImg<Tproc> &out)
  {
    kernelCPU_vPvMv(in,out);
  };//kernelCPU

  //! compution kernel for an iteration
  virtual void kernel(CImg<Tdata> &in,CImg<Tproc> &out)
  {
    kernelCPU(in,out);
  };//kernel

};//CDataProcessor_vPvMv

//! complex operation for CPU process (intended for GPU lambda)
/**
 * FMA: val*cst+cst, e.g. val * 2.1 + 123.45
**/
template<typename Tdata, typename Tproc>
void kernelCPU_vMcPc(CImg<Tdata> &in,CImg<Tproc> &out)
{
  out=in*2.1 + 123.45;
}//kernelCPU_vMcPc
//! complex operation for CPU process (intended for GPU lambda)
/**
 * FMA: val*cst+cst
**/
template<typename Tdata, typename Tproc=Tdata, typename Taccess=unsigned char>
class CDataProcessor_kernel : public CDataProcessor<Tdata,Tproc, Taccess>
{
public:
//  void *pKernel4CPU;
  CDataProcessor_kernel(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
//  , void *kernel4CPU(void)
  , bool do_check=false
  )
  : CDataProcessor<Tdata,Tproc, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check)
  {
    this->debug=true;
    this->class_name="CDataProcessor_kernel_vMcPc";
std::cout<<__FILE__<<"::"<<__func__<<"(...)"<<std::endl;
//    pKernel4CPU=kernel4CPU;
    this->check_locks(lock);
  }//constructor

  //! compution kernel for an iteration
  virtual void kernelCPU(CImg<Tdata> &in,CImg<Tproc> &out)
  {
//    *pKernelCPU(in,out);
    kernelCPU_vMcPc(in,out);
  };//kernelCPU

  virtual bool check_data(CImg<Tdata> &img, int i)
  {
//std::cout<<__FILE__<<"::"<<__func__<<"/"<<this->class_name<<"(...)"<<std::endl;
    if(this->do_check)
    {
      CImg<Tproc> imgt(img);
      kernelCPU(img,imgt);
      return (this->image==imgt);
    }//do_check
    return true;
  }//check_data

  //! compution kernel for an iteration
  virtual void kernel(CImg<Tdata> &in,CImg<Tproc> &out)
  {
    kernelCPU(in,out);
  };//kernel

};//CDataProcessor_kernel

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
class CDataProcessor_Max_Min : public CDataProcessor_kernel<Tdata,Tproc, Taccess>
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
  : CDataProcessor_kernel<Tdata,Tproc, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check)
  {
    this->debug=true;
    this->class_name="CDataProcessor_Max_Min";
std::cout<<__FILE__<<"::"<<__func__<<"(...)"<<std::endl;
    this->image.assign(1);//content: E only
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
 * - alpha=duty cicle
 * - fraction=
 *
 * \ref pageSchema "Signal schema" 
 *
**/
template<typename Tdata, typename Tproc=Tdata, typename Taccess=unsigned char>
class CDataProcessor_Trapeze : public CDataProcessor_kernel<Tdata,Tproc, Taccess>
{
public:
  int k, m, B, n, q, Tm, threshold;
  float alpha, fraction; 
  //! read processing parameters from CDL parameter file (as .nc)
  int Read_Paramaters (int &ks, int &ms, int &base, int &number, int &qDelay, int &Tpeak, int &th, float &alp, float &frac)
  {
  ///file name
  std::string fi="parameters.nc";//=cimg_option("-p","parameters.nc","comment");
  double Alpha, Fraction;
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
  if (error = fp.loadAttribute(attribute_name,ks)!=0){
    std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
    return error;
  }
  std::cout<<"  "<<attribute_name<<"="<<ks<<std::endl;
  ///m
  attribute_name="m";
  if (error = fp.loadAttribute(attribute_name,ms)!=0){
    std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
    return error;
  }
  std::cout<<"  "<<attribute_name<<"="<<ms<<std::endl;
  ///alpha
  attribute_name="alpha";
  if (error = fp.loadAttribute(attribute_name,Alpha)!=0){
    std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
    return error;
  }
  std::cout<<"  "<<attribute_name<<"="<<Alpha<<std::endl;
  ///n
  attribute_name="n";
  if (error = fp.loadAttribute(attribute_name,number)!=0){
    std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
    return error;
  }
  std::cout<<"  "<<attribute_name<<"="<<number<<std::endl;
  ///q
  attribute_name="q";
  if (error = fp.loadAttribute(attribute_name,qDelay)!=0){
    std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
    return error;
  }
  std::cout<<"  "<<attribute_name<<"="<<qDelay<<std::endl;
  //threshold
  attribute_name="threshold";
  if (error = fp.loadAttribute(attribute_name,th)!=0){
    std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
    return error;
  }
  std::cout<<"  "<<attribute_name<<"="<<th<<std::endl;
  ///fraction
  attribute_name="fraction";
  if (error = fp.loadAttribute(attribute_name,Fraction)!=0){
    std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
    return error;
  }
  std::cout<<"  "<<attribute_name<<"="<<Fraction<<std::endl;
  ///Tm
  attribute_name="Tm";
  if (error = fp.loadAttribute(attribute_name,Tpeak)!=0){
    std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
    return error;
  }
  std::cout<<"  "<<attribute_name<<"="<<Tpeak<<std::endl;

  process_name="graph";
  //load process variable
  error=fp.loadVar(process,&process_name);
  if(error){std::cerr<<"loadVar return "<< error <<std::endl;return error;}
  std::cout<<process_name<<"="<<process<<std::endl;
  ///B
  attribute_name="B";
  if (error = fp.loadAttribute(attribute_name,base)!=0){
    std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
    return error;
  }
  std::cout<<"  "<<attribute_name<<"="<<base<<std::endl;
  alp=Alpha;
  frac=Fraction;
  
  }//Read_Paramaters

  CDataProcessor_Trapeze(std::vector<omp_lock_t*> &lock
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessor_kernel<Tdata,Tproc, Taccess>(lock,wait_status,set_status,wait_statusR,set_statusR,do_check)
  {
    this->debug=true;
    this->class_name="CDataProcessor_Trapeze";
std::cout<<__FILE__<<"::"<<__func__<<"(...)"<<std::endl;
    Read_Paramaters(k,m,B,n,q,Tm,threshold, alpha,fraction);
    this->image.assign(1);//content: E only
    this->check_locks(lock);
  }//constructor
  //! fill the image with the filter
  virtual int trapezoidal_filter(CImg<Tdata> e, CImg<Tproc> &s, int ks, int ms, double alp, int decalage) 
  {
  //create a filter
    cimg_for_inX(s,decalage, s.width()-1,n)
    s(n)=2*s(n-1)-s(n-2) + e(n-1)-alp*e(n-2) -e(n-(ks+1)) \
			+alp*e(n-(ks+2))-e(n-(ks+ms+1))+alp*e(n-(ks+ms+2)) \
					+e(n-(2*ks+ms+1))-alp*e(n-(2*ks+ms+2));		
  }//trapezoidal_filter
  //! display the signal, the filter and the computation start
  #if cimg_display!=0
  virtual void Display(CImg<Tdata> in, CImg<Tproc> out, int decalage)
  {
	CImg<Tproc> imageC;
	imageC.assign(in.width(),1,1,3,0);
	imageC.get_shared_channel(0)+=in;
	imageC.get_shared_channel(1)+=out*in.max()/out.max(); // trapeze normalize
	cimg_for_inX(imageC,decalage,imageC.width(),i) imageC(i,0,0,2)=in.max();//begin of the trapeze computation
	imageC.display_graph("red = signal, green = filter, blue = trapezoidal computation");
  }//Display
  #endif // #if cimg_display

  //!fill the image with 2 discri and display it, return the position of the trigger
  virtual int Calcul_Ti(CImg<Tdata> e, int Tpeak,int th, double frac,double alp) 
  {
	CImg<Tproc> s(e.width());
	int delay = (3*Tpeak)/2;
	//Discri simple
	s(0)=0;
	cimg_for_inX(s,1,s.width(),n) s(n)=e(n)-alp*e(n-1);
	//Discri treshold		
	CImg<Tproc> imageDCF(s.width(),1,1,1, 0);
	cimg_for_inX(imageDCF,delay,s.width(),n) imageDCF(n)=s(n-delay)-frac*s(n);
	//find the position of the trigger
	int Ti;
	for (int i=0;s(i) < th; i++)
	{
	  Ti=i+1;
	}
        #if cimg_display!=0
	//display the graph
	CImg<Tproc> imageC;
	imageC.assign(s.width(),1,1,5, 0);
	imageC.get_shared_channel(0)+=s;
	imageC.get_shared_channel(1)+=imageDCF;
	imageC.get_shared_channel(2)+=th;
	imageC.get_shared_channel(3)+=e/e.max()*imageDCF.max();
	cimg_for_inX(imageC,Ti,imageC.width(),i) imageC(i,0,0,4)=imageDCF.max();		
	imageC.display_graph("red = discri simple, green = dCFD, blue = threshold, yellow = signal");
	#endif //#cimg_display
	return Ti;
  }//Calcul_Ti

  //! calculation of the energy based on the formula (peak-base)/number
  float Calculation_Energy(CImg<Tproc> trapeze, int Ti,int number, double qDelay)
  {
    //sum of the n baseline value
    int base=0;
    cimg_for_inX(trapeze,Ti-number, Ti,i) base+=trapeze(i);
    //sum of the n peak value
    int peak=0;
    cimg_for_inX(trapeze,Ti+qDelay, Ti+qDelay+number,i) peak+=trapeze(i);
    //print both sum and return the energy 
    std::cout<<"base="<<base/n<<std::endl;
    std::cout<<"peak="<<peak/n<<std::endl;
    return (peak-base)/number;
  }//Calculation_Energy
  #if cimg_display!=0
  //! display the filter in details (signal, baseline, delay and flat top)
  void Display_Trapeze_Paramaters(CImg<Tdata> in, int Ti,int number, double qDelay)
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
    int decalage = 2*k+m+2;
//! \todo [low] trapzoid container should be assigned once only
    CImg<Tproc> trapeze(in.width(),1,1,1, B);
    trapezoidal_filter(in,trapeze, k,m,alpha, decalage);
    #if cimg_display!=0
    Display(in, trapeze, decalage);
    #endif //#cimg_display
    int Ti=Calcul_Ti(in,Tm,threshold, fraction,alpha);
    std::cout<< "Trigger start= " << Ti  <<std::endl;
    #if cimg_display!=0
    Display_Trapeze_Paramaters(trapeze, Ti, n, q);
    #endif //#cimg_display
    float E=Calculation_Energy(trapeze, Ti, n, q);
    std::cout<< "Energy= " << E  <<std::endl;
    out(0)=E;
  };//kernelCPU_Trapeze

  //! compution kernel for an iteration
  virtual void kernelCPU(CImg<Tdata> &in,CImg<Tproc> &out)
  {
    kernelCPU_Trapeze(in,out);
  };//kernelCPU

};//CDataProcessor_Trapeze

#endif //NetCDF
	
#endif //_DATA_PROCESSOR_

