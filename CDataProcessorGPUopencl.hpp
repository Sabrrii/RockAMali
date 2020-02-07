#ifndef _DATA_PROCESSOR_GPU_OPENCL_
#define _DATA_PROCESSOR_GPU_OPENCL_

#include "CDataProcessorGPU.hpp"

//! complex operation with OpenCL including template types for GPU process
/**
 *  FMA: val * 2 + 123
 *  \note: Tdata and Tproc should be in the template source
**/
template<typename Tdata=unsigned int,typename Tproc=unsigned int, typename Taccess=unsigned char>
class CDataProcessorGPU_opencl_template : public CDataProcessorGPU_vMcPc_check<Tdata,Tproc, Taccess>
{
  compute::program program;
  compute::kernel  kernel;
  bool kernel_loaded;
//OpenCL function for this class
compute::program make_opencl_program(const compute::context& context)
{
//  const char source_with_template[] = BOOST_COMPUTE_STRINGIZE_SOURCE(
//  __kernel void vMcPc(__global const Tdata*input, int size, __global Tproc*output)
  const char source_with_template[] = BOOST_COMPUTE_STRINGIZE_SOURCE(
  __kernel void vMcPc(__global const unsigned int*input, int size, __global Tproc*output)
  {
    const int gid = get_global_id(0);
    output[gid]=input[gid]*2+123.45;
  }
  );//source
  //translate template
  //! \todo .
  std::string source=source_with_template;
  const std::string str_old="Tproc";
  const std::string str_new="float";
  //replace all str_old by str_new
  std::string::size_type pos = 0;
  while ( (pos=source.find(str_old, pos)) != std::string::npos )
  {
    source.replace(pos,str_old.size(), str_new);
    pos+=str_new.size();
  }//replace loop
  // create program
  return compute::program::build_with_source(source,context);
}//make_opencl_program

public:
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
    this->debug=true;
    this->class_name="CDataProcessorGPU_openclT_vMcPc";
    this->check_locks(lock);
    //OpenCL framework
    program=make_opencl_program(this->ctx);
    kernel_loaded=false;
  }//constructor

  //! compution kernel for an iteration (compution=copy, here)
  virtual void kernelGPU(compute::vector<Tdata> &in,compute::vector<Tproc> &out)
  {
    if(!kernel_loaded)
    {//load kernel
      kernel=compute::kernel(program, "vMcPc");
      kernel.set_arg(0,this->device_vector_in.get_buffer());
      kernel.set_arg(1,(int)this->device_vector_in.size());
      kernel.set_arg(2,this->device_vector_out.get_buffer());
      kernel_loaded=true;
    }//load kernel once
    //compute
    using compute::uint_;
    uint_ tpb=16;
    uint_ workSize=this->device_vector_in.size();
    this->queue.enqueue_1d_range_kernel(kernel,0,workSize,tpb);
  };//kernelGPU

};//CDataProcessorGPU_opencl_template

#endif //_DATA_PROCESSOR_GPU_OPENCL_

