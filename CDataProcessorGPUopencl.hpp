#ifndef _DATA_PROCESSOR_GPU_OPENCL_
#define _DATA_PROCESSOR_GPU_OPENCL_

#include "CDataProcessorGPU.hpp"

//! complex operation with OpenCL including template types for GPU process
/**
 *  FMA: val * 2.1 + 123.45
 *  \note: Tdata and Tproc could be in the template source
**/
template<typename Tdata=unsigned int,typename Tproc=unsigned int, typename Taccess=unsigned char>
class CDataProcessorGPU_opencl_template : public CDataProcessorGPU_vMcPc_check<Tdata,Tproc, Taccess>
{
public:
  compute::program program;
  compute::kernel  oclKernel;
  bool kernel_loaded;
//OpenCL function for this class
  //! OpenCL kernel name
  std::string kernel_name;
  //! OpenCL source string (with template)
  std::string source_with_template;
//! OpenCL source (with template)
/**
 * template types must be either \c Tdata or \c Tproc
 * \note this function should redefined in inherited class
**/
virtual void define_opencl_source()
{
  kernel_name="vMcPc";
  source_with_template=BOOST_COMPUTE_STRINGIZE_SOURCE(
  __kernel void vMcPc(__global const Tdata*input, int size, __global Tproc*output)
  {
    const int gid = get_global_id(0);
    const Tproc mul=2.1;
    const Tproc cst=123.45;
    output[gid]=input[gid]*mul+cst;
  }
  );//source with template
}//define_opencl_source
//! OpenCL program from source (with template)
/**
 * template types: \c Tdata or \c Tproc only will be tranlated, not more !
**/
virtual compute::program make_opencl_program(const compute::context& context)
{
  //init. source
  define_opencl_source();
  //translate template
  std::string source=source_with_template;
  std::vector<std::string> str_old;str_old.push_back(    "Tdata");              str_old.push_back(    "Tproc");
  std::vector<std::string> str_new;str_new.push_back(CImg<Tdata>::pixel_type());str_new.push_back(CImg<Tproc>::pixel_type());
  //! \bug [template in OpenCL] CImg<T>::pixel_type() is almost ok, except for "long int" giving "int64" (and may be more !?)
  for(unsigned int i=0;i<str_old.size();++i)
  {
    //replace all str_old by str_new
    std::string::size_type pos=0;
    while( (pos=source.find(str_old[i], pos)) != std::string::npos )
    {
      source.replace(pos,str_old[i].size(), str_new[i]);
      pos+=str_new[i].size();
    }//replace loop
  }//for loop
std::cout<<"source:"<<std::endl<<"\""<<source<<std::endl<<"\""<<std::endl<<std::flush;
  // create program
  return compute::program::build_with_source(source,context);
}//make_opencl_program

  CDataProcessorGPU_opencl_template(std::vector<omp_lock_t*> &lock
  , compute::device device, int VECTOR_SIZE
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessorGPU_vMcPc_check<Tdata,Tproc, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check)
  {
//    this->debug=true;
    this->check_locks(lock);
    //OpenCL framework
    program=make_opencl_program(this->ctx);
    this->class_name="CDataProcessorGPU_openclT_"+kernel_name;
    kernel_loaded=false;
  }//constructor

  //! compution kernel for an iteration
  virtual void kernelGPU(compute::vector<Tdata> &in,compute::vector<Tproc> &out)
  {
    if(!kernel_loaded)
    {//load kernel
      oclKernel=compute::kernel(program,kernel_name.c_str());
      oclKernel.set_arg(0,this->device_vector_in.get_buffer());
      oclKernel.set_arg(1,(int)this->device_vector_in.size());
      oclKernel.set_arg(2,this->device_vector_out.get_buffer());
      kernel_loaded=true;
    }//load kernel once
    //compute
    using compute::uint_;
    uint_ tpb=16;
    uint_ workSize=this->device_vector_in.size();
    this->queue.enqueue_1d_range_kernel(oclKernel,0,workSize,tpb);
  };//kernelGPU

};//CDataProcessorGPU_opencl_template


//! 4 complex operation with OpenCL including template types for GPU process
/**
 *  FMA by 4: val * 2.1 + 123.45
 *  Tdata4 and Tproc4 should same type as Tdata and Tproc
 *  \note: Tdata and Tproc only could be in the template source
**/
template<typename Tdata=unsigned int,typename Tproc=unsigned int, typename Taccess=unsigned char
, typename Tdata4=compute::uint4_, typename Tproc4=compute::float4_
>
class CDataProcessorGPU_opencl_T4 : public CDataProcessorGPU_opencl_template<Tdata,Tproc, Taccess>
{
public:
  // create vectors on the device
  compute::vector<Tdata4> device_vector_in4;
  compute::vector<Tproc4> device_vector_out4;
  CImg<Tdata4> in4;
  CImg<Tproc4> out4;

//! OpenCL source (with template)
/**
 * template types must be either \c Tdata or \c Tproc
 * \note this function should redefined in inherited class
**/
virtual void define_opencl_source()
{
  this->kernel_name="vMcPc4";
  this->source_with_template=BOOST_COMPUTE_STRINGIZE_SOURCE(
  __kernel void vMcPc4(__global const Tdata*input, int size, __global Tproc*output)
  {
    const int gid = get_global_id(0)*4;
    const Tproc mul=2.1;
    const Tproc cst=123.45;
    output[gid]  =input[gid]  *mul+cst;
    output[gid+1]=input[gid+1]*mul+cst;
    output[gid+2]=input[gid+2]*mul+cst;
    output[gid+3]=input[gid+3]*mul+cst;
  }
  );//source with template
}//define_opencl_source

  CDataProcessorGPU_opencl_T4(std::vector<omp_lock_t*> &lock
  , compute::device device, int VECTOR_SIZE
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessorGPU_opencl_template<Tdata,Tproc, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check)
  , device_vector_in4(VECTOR_SIZE/4, this->ctx), device_vector_out4(VECTOR_SIZE/4, this->ctx)
  {
//    this->debug=true;
    this->check_locks(lock);
    in4._width=out4._width=VECTOR_SIZE/4;
    in4._height=out4._height=1;
    in4._depth=out4._depth=1;
    in4._spectrum=out4._spectrum=1;
    //OpenCL framework
    this->program=this->make_opencl_program(this->ctx);
    this->class_name="CDataProcessorGPU_openclT4_"+this->kernel_name;
    this->kernel_loaded=false;
  }//constructor

  //! compution kernel for an iteration
  virtual void kernelGPU4(compute::vector<Tdata4> &in,compute::vector<Tproc4> &out)
  {
    if(!this->kernel_loaded)
    {//load kernel
      this->oclKernel=compute::kernel(this->program,this->kernel_name.c_str());
      this->oclKernel.set_arg(0,this->device_vector_in4.get_buffer());
      this->oclKernel.set_arg(1,(int)this->device_vector_in4.size());
      this->oclKernel.set_arg(2,this->device_vector_out4.get_buffer());
      this->kernel_loaded=true;
    }//load kernel once
    //compute
    using compute::uint_;
    uint_ tpb=16;
    uint_ workSize=this->device_vector_in4.size();
    this->queue.enqueue_1d_range_kernel(this->oclKernel,0,workSize,tpb);
  };//kernelGPU

  //! compution kernel for an iteration
  virtual void kernel(CImg<Tdata> &in,CImg<Tproc> &out)
  {
    ///share data
    in4._data=(Tdata4*)in.data();
    out4._data=(Tproc4*)out.data();
    //copy CPU to GPU
   #ifdef DO_GPU_PROFILING
    this->future=compute::copy_async
   #else
    compute::copy
   #endif //DO_GPU_PROFILING
    (in4.begin(),in4.end(), device_vector_in4.begin(), this->queue);
    //compute
    kernelGPU4(device_vector_in4,device_vector_out4);
    //copy GPU to CPU
    compute::copy(device_vector_out4.begin(),device_vector_out4.end(), out4.begin(), this->queue);
    //wait for completion
    this->queue.finish();
   #ifdef DO_GPU_PROFILING
    this->kernel_elapsed_time();
   #endif //DO_GPU_PROFILING
    ///unshare data
    in4._data=NULL;
    out4._data=NULL;
  };//kernel

};//CDataProcessorGPU_opencl_T4

//! 4 complex operation with OpenCL including template types for GPU process
/**
 *  FMA by 4.xyzw: val * 2.1 + 123.45
 *  Tdata4 and Tproc4 should same type as Tdata and Tproc
 *  \note: Tdata and Tproc only could be in the template source
**/
template<typename Tdata=unsigned int,typename Tproc=unsigned int, typename Taccess=unsigned char
, typename Tdata4=compute::uint4_, typename Tproc4=compute::float4_
>
class CDataProcessorGPU_opencl_T4xyzw : public CDataProcessorGPU_opencl_T4<Tdata,Tproc, Taccess>
{
public:
//! OpenCL source (with template)
/**
 * template types must be either \c Tdata or \c Tproc
**/
virtual void define_opencl_source()
{
  this->kernel_name="vMcPc4xyzw";
  this->source_with_template=BOOST_COMPUTE_STRINGIZE_SOURCE(
  __kernel void vMcPc4xyzw(__global const Tdata*input, int size, __global Tproc*output)
  {
    const int gid = get_global_id(0)*4;
    const Tproc4 mul=(Tproc4)(2.1);
    const Tproc4 cst=(Tproc4)(123.45);
    Tproc4 in;
    Tproc4 out;
    in.x=(Tproc)(input[gid]);
    in.y=(Tproc)(input[gid+1]);
    in.z=(Tproc)(input[gid+2]);
    in.w=(Tproc)(input[gid+3]);
    out=in*mul+cst;
    output[gid]  =out.x;
    output[gid+1]=out.y;
    output[gid+2]=out.z;
    output[gid+3]=out.w;
  }
  );//source with template
}//define_opencl_source

  CDataProcessorGPU_opencl_T4xyzw(std::vector<omp_lock_t*> &lock
  , compute::device device, int VECTOR_SIZE
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessorGPU_opencl_T4<Tdata,Tproc, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check)
  {
//    this->debug=true;
    this->check_locks(lock);
    //OpenCL framework
    this->program=this->make_opencl_program(this->ctx);
    this->class_name="CDataProcessorGPU_openclT4_"+this->kernel_name;
    this->kernel_loaded=false;
  }//constructor

};//CDataProcessorGPU_opencl_T4xyzw

//! 4 complex operation with OpenCL including template types for GPU process
/**
 *  FMA by 4.load fma store: val * 2.1 + 123.45
 *  Tdata4 and Tproc4 should same type as Tdata and Tproc
 *  \note: Tdata and Tproc only could be in the template source
**/
template<typename Tdata=unsigned int,typename Tproc=unsigned int, typename Taccess=unsigned char
, typename Tdata4=compute::uint4_, typename Tproc4=compute::float4_
>
class CDataProcessorGPU_opencl_T4ls_fma : public CDataProcessorGPU_opencl_T4<Tdata,Tproc, Taccess>
{
public:
//! OpenCL source (with template)
/**
 * template types must be either \c Tdata or \c Tproc
**/
virtual void define_opencl_source()
{
  this->kernel_name="vMcPc4ls_fma";
  this->source_with_template=BOOST_COMPUTE_STRINGIZE_SOURCE(
  __kernel void vMcPc4ls_fma(__global const Tdata*input, int size, __global Tproc*output)
  {
    const int gid = get_global_id(0);
    const Tproc4 mul=(Tproc4)(2.1);
    const Tproc4 cst=(Tproc4)(123.45);
    uint4 uin=vload4(gid,input);
    Tproc4 in=convert_Tproc4(uin);
    Tproc4 out=fma(in,mul,cst);
    vstore4(out,gid,output);
  }
  );//source with template
}//define_opencl_source

  CDataProcessorGPU_opencl_T4ls_fma(std::vector<omp_lock_t*> &lock
  , compute::device device, int VECTOR_SIZE
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_status=CDataAccess::STATUS_FILLED
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_status=CDataAccess::STATUS_PROCESSED
  , CDataAccess::ACCESS_STATUS_OR_STATE wait_statusR=CDataAccess::STATUS_FREE
  , CDataAccess::ACCESS_STATUS_OR_STATE  set_statusR=CDataAccess::STATUS_FILLED
  , bool do_check=false
  )
  : CDataProcessorGPU_opencl_T4<Tdata,Tproc, Taccess>(lock,device,VECTOR_SIZE,wait_status,set_status,wait_statusR,set_statusR,do_check)
  {
//    this->debug=true;
    this->check_locks(lock);
    //OpenCL framework
    this->program=this->make_opencl_program(this->ctx);
    this->class_name="CDataProcessorGPU_openclT4_"+this->kernel_name;
    this->kernel_loaded=false;
  }//constructor

};//CDataProcessorGPU_opencl_T4ls_fma

#endif //_DATA_PROCESSOR_GPU_OPENCL_

