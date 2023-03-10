#ifndef _DATA_PROCESSOR_GPU_ENERGY_
#define _DATA_PROCESSOR_GPU_ENERGY_

#include "CDataProcessorGPU.hpp"
#include "CDataProcessor_energy.hpp"

#ifdef DO_NETCDF

//!  \todo [low] set template for opencl code \see CDataProcessorGPU_opencl_template
//!  \todo [medium] setup kernel_name="vMcPcI" as in \c CDataProcessorGPU_opencl_template

//! discri calculation GPU process 
/**
 * computation of the discri
 * return the value of energy
**/
template<typename Tdata=unsigned int,typename Tproc=float, typename Taccess=unsigned char>
class CDataProcessorGPU_discri_opencl : public CDataProcessorGPU<Tdata,Tproc, Taccess>, public CData_Filter<Tdata,Tproc>
{

//OpenCL function for this class
compute::program make_opencl_program(const compute::context& context)
{
  const char source[] = BOOST_COMPUTE_STRINGIZE_SOURCE(
  __kernel void discri(__global const unsigned int*input, int size, __global float*output, float alpha)
  {   
    const int gid = get_global_id(0);
    if ( gid == 0) 
    {
	output[gid] = 0;
    } 
    else
    {
        output[gid]=input[gid]-alpha*input[gid-1];
    }
  }//discri
  );//source
  // create program
  return compute::program::build_with_source(source,context);
}//make_opencl_program

public:
  bool kernel_loaded;
  compute::program program;
  compute::kernel  ocl_kernel;
  //temporary processing image
  bool image_assigned;
  CImg<Tproc> discri;
  CImg<Tproc> trapezoid;

  CDataProcessorGPU_discri_opencl(std::vector<omp_lock_t*> &lock
  , compute::device device, int VECTOR_SIZE
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessorGPU<Tdata,Tproc, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check)
  {
//    this->debug=true;
    this->class_name="CDataProcessorGPU_discri_opencl";
    this->check_locks(lock);
    this->image.assign(1);//content: E only
    ///read paramaters in NetCDF file
    this->read_parameters_and_setup();
    ///OpenCL framework
    program=make_opencl_program(this->ctx);
    kernel_loaded=false;
    image_assigned=false;
  }//constructor

  //! compution kernel for an iteration (compution=copy, here)
  virtual void kernelGPU(compute::vector<Tdata> &in,compute::vector<Tproc> &out)
  {
    if(!kernel_loaded)
    {//load kernel
      ocl_kernel=compute::kernel(program, "discri");
      ocl_kernel.set_arg(0,this->device_vector_in.get_buffer());
      ocl_kernel.set_arg(1,(int)this->device_vector_in.size());
      ocl_kernel.set_arg(2,this->device_vector_out.get_buffer());
      ocl_kernel.set_arg(3,this->alpha);
      kernel_loaded=true;
    }//load kernel once
    //compute
    using compute::uint_;
    uint_ tpb=16;
    uint_ workSize=this->device_vector_in.size();
    this->queue.enqueue_1d_range_kernel(ocl_kernel,0,workSize,tpb);
  };//kernelGPU

  //! compution kernel for an iteration
  virtual void kernel(CImg<Tdata> &in,CImg<Tproc> &out)
  {
    /// discri computation on GPU      
    if(!image_assigned) {discri.assign(in.width());/*image_assigned=true;*/}
    //copy CPU to GPU
    compute::copy(in.begin(), in.end(), this->device_vector_in.begin(), this->queue);
    //compute
    kernelGPU(this->device_vector_in,this->device_vector_out);
    //copy GPU to CPU
    compute::copy(this->device_vector_out.begin(), this->device_vector_out.end(), discri.begin(), this->queue);
    kernel_Energy(in,discri,out);
  };//kernel

  virtual void kernel_Energy(CImg<Tdata> &in, CImg<Tproc> &discri, CImg<Tproc> &out)
  {
    ///Trapezoidal computation on CPU    
    if(!image_assigned) {trapezoid.assign(in.width());image_assigned=true;}
    this->trapezoidal_filter(in,trapezoid, this->k,this->m,this->alpha, this->decalage);
    //wait for GPU completion
    this->queue.finish();
//! \todo add DO_GPU_PROFILING
    ///find trigger on CPU
    int Ti=this->Calcul_Ti(discri,this->threshold);
    std::cout<<"Trigger value :"<<Ti<<std::endl;
    /// energy computation on CPU    
    float E=this->Calculation_Energy(trapezoid, Ti, this->n, this->q);
    std::cout<< "Energy= " << E  <<std::endl;
    ///store energy in image
    out(0)=E;//content: E only
  }//kernel_Energy

#ifdef DO_NETCDF
  virtual void set_var_unit_long_names(std::vector<std::string> &var_unit_long_names)
  {
/** /
    var_unit_long_names.push_back("signal");
    var_unit_long_names.push_back("digit*");
//    var_unit_long_names.push_back("CPU trapezoid signal");
    var_unit_long_names.push_back("GPU discri signal");
/ **/
    var_unit_long_names.push_back("E");
    var_unit_long_names.push_back("digit*");
    var_unit_long_names.push_back("energy");
/**/
  }//set_var_unit_long_names
#endif //NetCDF

};//CDataProcessorGPU_discri_opencl

//! discri calculation GPU process 
/**
 * computation of the discri
 * return the value of energy
 * make the computation on SIMD_2
**/
template<typename Tdata=unsigned int,typename Tproc=unsigned int, typename Taccess=unsigned char>
class CDataProcessorGPU_discri_opencl_int2 : public CDataProcessorGPU_discri_opencl<Tdata,Tproc, Taccess>
{
  typedef compute::uint2_  Tdata2;
  typedef compute::float2_  Tproc2;
  compute::vector<Tdata2> device_vector_in2;
  compute::vector<Tproc2> device_vector_out2;
  CImg<Tdata2>in2;
  CImg<Tproc2>out2;
//OpenCL function for this class
compute::program make_opencl_program(const compute::context& context)
{
  const char source[] = BOOST_COMPUTE_STRINGIZE_SOURCE(
  __kernel void discri(__global const unsigned int*input, int size, __global float*output, float alpha)
  {
    const int gid = get_global_id(0)*2;
    if ( gid == 0) 
    {
	output[0] = 0;
	output[1] = input[1]-alpha*input[0];
    } 
    else
    {
        output[gid]=input[gid]-alpha*input[gid-1];
        output[gid+1]=input[gid+1]-alpha*input[gid];
    }    
  }//discri
  __kernel void discri_fma(__global const unsigned int*input, int size, __global float*output, float alpha)
  {
    const int gid = get_global_id(0)*2;
    if ( gid == 0) 
    {
	output[0] = 0;
	//output[1] = input[1]-alpha*input[0];
	//output[1] = -alpha*input[0] + input[1];
	output[1]=fma(-alpha,input[0], input[1]);
    } 
    else
    {	
	output[gid]=fma(-alpha,input[gid-1], input[gid]);
	output[gid+1]=fma(-alpha,input[gid], input[gid+1]);
    }    
  }//discri_fma
  __kernel void discri_ls_fma(__global const unsigned int*input, int size, __global float*output, float alpha)
  {
    const int gid = get_global_id(0);//*2;
    const float2 alpha2=(-alpha,-alpha);
    // load gid -1 data
    uint   in_  =(gid==0)?input[0]:input[(gid*2)-1];
    float  fn_  =(gid==0)?((float)(in_)/alpha):(float)in_;
    // load 4 gid data
    uint2  in2  =vload2(gid,input);
    float2 fn2  =convert_float2(in2);
    // gid - 1 data
    float2 fn2_ =(fn_,fn2.x);
    // discri
    float2 out2 =fma(alpha2,fn2,fn2_);
    // storage
    vstore2(out2, gid, output);
  }//discri_ls_fma
  );//source
  // create program
  return compute::program::build_with_source(source,context);
}//make_opencl_program

public:

  CDataProcessorGPU_discri_opencl_int2(std::vector<omp_lock_t*> &lock
  , compute::device device, int VECTOR_SIZE
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessorGPU_discri_opencl<Tdata,Tproc, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check)
   ,device_vector_in2(VECTOR_SIZE/2, this->ctx),device_vector_out2(VECTOR_SIZE/2, this->ctx)
  {
//    this->debug=true;
    this->class_name="CDataProcessorGPU_discri_opencl_int2";
    this->check_locks(lock);
    //OpenCL framework
    this->program=make_opencl_program(this->ctx);
    this->kernel_loaded=false;
    in2._width=out2._width=VECTOR_SIZE/2;
    in2._height=out2._height=1;
    in2._depth=out2._depth=1;
    in2._spectrum=out2._spectrum=1;
  }//constructor

  //! compution kernel for an iteration
  virtual void kernel(CImg<Tdata> &in,CImg<Tproc> &out)
  {
    /// discri computation on GPU      
    if(!(this->image_assigned)) {this->discri.assign(in.width());/*image_assigned=true;*/}
    ///share data in2=in and out2=this->discri (and vice versa)
    in2._data=(Tdata2*)in.data();
    out2._data=(Tproc2*)((this->discri).data());
    //copy CPU to GPU
   #ifdef DO_GPU_PROFILING
    this->future=compute::copy_async
   #else
    compute::copy
   #endif //DO_GPU_PROFILING
    (in2.begin(), in2.end(), device_vector_in2.begin(), this->queue);
    //compute
    kernelGPU2(device_vector_in2,device_vector_out2);
    //copy GPU to CPU
    compute::copy(device_vector_out2.begin(), device_vector_out2.end(), out2.begin(), this->queue);
/*
    //wait for completion
    this->queue.finish();
   #ifdef DO_GPU_PROFILING
    this->kernel_elapsed_time();
   #endif //DO_GPU_PROFILING
*/
    this->kernel_Energy(in,this->discri,out);
/**/
   #ifdef DO_GPU_PROFILING
    this->kernel_elapsed_time();
   #endif //DO_GPU_PROFILING
/**/
    ///unshare data
    in2._data=NULL;
    out2._data=NULL;
  };//kernel

  //! compution kernel for an iteration (compution=copy, here)
  virtual void kernelGPU2(compute::vector<Tdata2> &in,compute::vector<Tproc2> &out)
  {
    if(!this->kernel_loaded)
    {//load kernel
      this->ocl_kernel=compute::kernel(this->program,"discri_ls_fma");
      this->ocl_kernel.set_arg(0,this->device_vector_in2.get_buffer());
      this->ocl_kernel.set_arg(1,(int)this->device_vector_in2.size());
      this->ocl_kernel.set_arg(2,this->device_vector_out2.get_buffer());
      this->ocl_kernel.set_arg(3,this->alpha);
      this->kernel_loaded=true;
    }//load kernel once
    //compute
    using compute::uint_;
    uint_ tpb=16;
    uint_ workSize=this->device_vector_in2.size();
    this->queue.enqueue_1d_range_kernel(this->ocl_kernel,0,workSize,tpb);
  };//kernelGPU2

};//CDataProcessorGPU_discri_opencl_int2

//! discri calculation GPU process 
/**
 * computation of the discri
 * return the value of energy
 * make the computation on SIMD_4
**/
template<typename Tdata=unsigned int,typename Tproc=unsigned int, typename Taccess=unsigned char>
class CDataProcessorGPU_discri_opencl_int4 : public CDataProcessorGPU_discri_opencl<Tdata,Tproc, Taccess>
{  
  typedef compute::uint4_  Tdata4;
  typedef compute::float4_  Tproc4;
  compute::vector<Tdata4> device_vector_in4;
  compute::vector<Tproc4> device_vector_out4;
  CImg<Tdata4>in4;
  CImg<Tproc4>out4;
  //OpenCL function for this class
  compute::program make_opencl_program(const compute::context& context)
{
  const char source[] = BOOST_COMPUTE_STRINGIZE_SOURCE(
  __kernel void discri_fma(__global const unsigned int*input, int size, __global float*output, float alpha)
  {
    const int gid = get_global_id(0)*4;
    if ( gid == 0) 
    {
	output[0] = 0;
	output[1]=fma(-alpha,input[0], input[1]);
	output[2]=fma(-alpha,input[1], input[2]);
	output[3]=fma(-alpha,input[2], input[3]);
    }
    else
    {
	output[gid]=fma(-alpha,input[gid-1], input[gid]);
	output[gid+1]=fma(-alpha,input[gid], input[gid+1]);
	output[gid+2]=fma(-alpha,input[gid+1], input[gid+2]);
	output[gid+3]=fma(-alpha,input[gid+2], input[gid+3]);
    }
  }//discri_fma
  __kernel void discri_ls_fma(__global const unsigned int*input, int size, __global float*output, float alpha)
  {
    const int gid = get_global_id(0);//*4;
    const float4 alpha4=(-alpha,-alpha,-alpha,-alpha);
    // load gid -1 data
    uint   in_  =(gid==0)?input[0]:input[(gid*4)-1];
    float  fn_  =(gid==0)?((float)(in_)/alpha):(float)in_;
    // load 4 gid data
    uint4  in4  =vload4(gid,input);
    float4 fn4  =convert_float4(in4);
    // gid - 1 data
    float4 fn4_ =(fn_,fn4.x,fn4.y,fn4.z);//(fn_,fn4.xyz);
    // discri
    float4 out4 =fma(alpha4,fn4,fn4_);
    // storage
    vstore4(out4, gid, output);
  }//discri_ls_fma
  );//source
  // create program
  std::cout<<"source = "<<source<<std::endl;
  return compute::program::build_with_source(source,context);
}//make_opencl_program

public:

  CDataProcessorGPU_discri_opencl_int4(std::vector<omp_lock_t*> &lock
  , compute::device device, int VECTOR_SIZE
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessorGPU_discri_opencl<Tdata,Tproc, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check)
   ,device_vector_in4(VECTOR_SIZE/4, this->ctx),device_vector_out4(VECTOR_SIZE/4, this->ctx)
  {
//    this->debug=true;
    this->class_name="CDataProcessorGPU_discri_opencl_int4";
    this->check_locks(lock);
    //OpenCL framework
    this->program=make_opencl_program(this->ctx);
    this->kernel_loaded=false;
    in4._width=out4._width=VECTOR_SIZE/4;
    in4._height=out4._height=1;
    in4._depth=out4._depth=1;
    in4._spectrum=out4._spectrum=1;
  }//constructor

  //! compution kernel for an iteration
  virtual void kernel(CImg<Tdata> &in,CImg<Tproc> &out)
  {
    /// discri computation on GPU      
    if(!(this->image_assigned)) {this->discri.assign(in.width());/*image_assigned=true;*/}
    ///share data in4=in and out4=this->discri (and vice versa)
    in4._data=(Tdata4*)in.data();
    out4._data=(Tproc4*)((this->discri).data());
    //copy CPU to GPU
   #ifdef DO_GPU_PROFILING
    this->future=compute::copy_async
   #else
    compute::copy
   #endif //DO_GPU_PROFILING
    (in4.begin(), in4.end(), device_vector_in4.begin(), this->queue);
    //compute
    kernelGPU4(device_vector_in4,device_vector_out4);
    //copy GPU to CPU
    compute::copy(device_vector_out4.begin(), device_vector_out4.end(), out4.begin(), this->queue);
/*
    //wait for completion
    this->queue.finish();
   #ifdef DO_GPU_PROFILING
    this->kernel_elapsed_time();
   #endif //DO_GPU_PROFILING
*/
    this->kernel_Energy(in,this->discri,out);
/**/
   #ifdef DO_GPU_PROFILING
    this->kernel_elapsed_time();
   #endif //DO_GPU_PROFILING
/**/
    ///unshare data
    in4._data=NULL;
    out4._data=NULL;
  };//kernel

  //! compution kernel for an iteration (compution=copy, here)
  virtual void kernelGPU4(compute::vector<Tdata4> &in,compute::vector<Tproc4> &out)
  {
    if(!this->kernel_loaded)
    {//load kernel
      this->ocl_kernel=compute::kernel(this->program,"discri_ls_fma");
      this->ocl_kernel.set_arg(0,this->device_vector_in4.get_buffer());
      this->ocl_kernel.set_arg(1,(int)this->device_vector_in4.size());
      this->ocl_kernel.set_arg(2,this->device_vector_out4.get_buffer());
      this->ocl_kernel.set_arg(3,this->alpha);
      this->kernel_loaded=true;
    }//load kernel once
    //compute
    using compute::uint_;
    uint_ tpb=16;
    uint_ workSize=this->device_vector_in4.size();
    this->queue.enqueue_1d_range_kernel(this->ocl_kernel,0,workSize,tpb);
  };//kernelGPU4

};//CDataProcessorGPU_discri_opencl_int4

#endif //NetCDF

#endif //_DATA_PROCESSOR_GPU_ENERGY_

