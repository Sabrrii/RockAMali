#ifndef _DATA_PROCESSOR_GPU_VMCPC_
#define _DATA_PROCESSOR_GPU_VMCPC_

#include "CDataProcessorGPU.hpp"
#include "CDataProcessor_vMcPc.hpp"

//! vMcPc operation for GPU process
/**
 *  virtual check class
 *  FMA: _1 * 2 + 123
**/
template<typename Tdata,typename Tproc, typename Taccess=unsigned char>
class CDataProcessorGPU_vMcPc_check : public CDataProcessorGPU<Tdata,Tproc, Taccess>
{
public:
  CDataProcessorGPU_vMcPc_check(std::vector<omp_lock_t*> &lock
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
    this->class_name="CDataProcessorGPU_vMcPc_check";
    this->check_locks(lock);
  }//constructor

  virtual bool check_data(CImg<Tdata> &img, int i)
  {
//std::cout<<__FILE__<<"::"<<__func__<<"/"<<this->class_name<<"(...)"<<std::endl;
    if(this->do_check)
    {
      const Tproc epsilon=0.001;
      CImg<Tproc> imgt;
      kernelCPU_vMcPc(img,imgt);
//imgt.print("img check",false);
//this->image.print("img GPU",false);
      const Tproc error=(this->image-imgt).abs().max();
      const bool no_error=(error<epsilon);
      if(!no_error) std::cout<<__FILE__<<"::"<<__func__<<"/"<<this->class_name<<"(...) max. error="<<error<<std::endl;
      return no_error;
    }//do_check
    return true;
  }//check_data
/**
  //! compution kernel for an iteration (compution=copy, here)
  virtual void kernelGPU(compute::vector<Tdata> &in,compute::vector<Tdata> &out)
  {
    //compute with lambda
    using compute::lambda::_1;
    compute::transform(in.begin(), in.end(), out.begin(),
      _1 * 2 + 123 , this->queue);
  };//kernelGPU
**/
};//CDataProcessorGPU_vMcPc_check

//! complex operation with lambda for GPU process
/**
 *  FMA: _1 * 2.1 + 123.45
**/
template<typename Tdata,typename Tproc, typename Taccess=unsigned char>
class CDataProcessorGPU_lambda : public CDataProcessorGPU_vMcPc_check<Tdata,Tproc, Taccess>
{
public:
  CDataProcessorGPU_lambda(std::vector<omp_lock_t*> &lock
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
    this->class_name="CDataProcessorGPU_vMcPc_lambda";
    this->check_locks(lock);
  }//constructor

  //! compution kernel for an iteration (compution=copy, here)
  virtual void kernelGPU(compute::vector<Tdata> &in,compute::vector<Tproc> &out)
  {
    //compute with lambda
    using compute::lambda::_1;
    compute::transform(in.begin(), in.end(), out.begin(),
      _1 * 2.1 + 123.45 , this->queue);
  };//kernelGPU

};//CDataProcessorGPU_lambda

//! complex operation with function using lambda for GPU process
/**
 *  FMA: _1 * 2.1 + 123.45
**/
template<typename Tdata,typename Tproc, typename Taccess=unsigned char>
class CDataProcessorGPU_function_lambda : public CDataProcessorGPU_vMcPc_check<Tdata,Tproc, Taccess>
{
public:
  CDataProcessorGPU_function_lambda(std::vector<omp_lock_t*> &lock
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
    this->class_name="CDataProcessorGPU_function_vMcPc_lambda";
    this->check_locks(lock);
  }//constructor

  //! compution kernel for an iteration (compution=copy, here)
  virtual void kernelGPU(compute::vector<Tdata> &in,compute::vector<Tproc> &out)
  {
    //compute with lambda
    using compute::lambda::_1;
    compute::function<Tproc(Tdata)> vMcPc = _1 * 2.1 + 123.45;
    compute::transform(in.begin(), in.end(), out.begin(),
      vMcPc , this->queue);
  };//kernelGPU

};//CDataProcessorGPU_function_lambda

//! complex operation with closure for GPU process
/**
 *  FMA: _1 * 2.1 + 123.45
 *  \note: more complex compution in boost::compute test, e.g. triangle_area in test/test_closure.cpp
**/
template<typename Tdata,typename Tproc, typename Taccess=unsigned char>
class CDataProcessorGPU_closure : public CDataProcessorGPU_vMcPc_check<Tdata,Tproc, Taccess>
{
public:
  CDataProcessorGPU_closure(std::vector<omp_lock_t*> &lock
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
    this->class_name="CDataProcessorGPU_vMcPc_closure";
    this->check_locks(lock);
  }//constructor

  //! compution kernel for an iteration (compution=copy, here)
  virtual void kernelGPU(compute::vector<Tdata> &in,compute::vector<Tproc> &out)
  {
    //compute with closure
    Tproc mul = 2.1;
    Tproc cst = 123.45;
    BOOST_COMPUTE_CLOSURE(Tproc, vMcPc, (Tdata x), (mul, cst),
    {
        return x * mul + cst;
    });

    compute::transform(in.begin(), in.end(), out.begin()
      , vMcPc
      , this->queue);

  };//kernelGPU

};//CDataProcessorGPU_closure

//! complex operation with function for GPU process
/**
 *  FMA: val * 2.1 + 123.45
 *  \warning: Tdata should be unsigned int as function from source lock type
 *  \note: function is static code (for compute::make_function_from_source, in constructor)
**/
template<typename Tdata=unsigned int,typename Tproc=float, typename Taccess=unsigned char>
class CDataProcessorGPU_function : public CDataProcessorGPU_vMcPc_check<Tdata,Tproc, Taccess>
{
  compute::function<Tproc (Tdata)> *vMcPc;
  void make_OpenCL_function()
  {
    static compute::function<Tproc (Tdata)> tmp=compute::make_function_from_source<Tproc (Tdata)>(
        "vMcPc",
        "float vMcPc(unsigned int x) { return x *2.1f + 123.45f; }"
    );
    vMcPc=&tmp;
  }//make_OpenCL_function
public:

  CDataProcessorGPU_function(std::vector<omp_lock_t*> &lock
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
    this->class_name="CDataProcessorGPU_function_vMcPc_uInt";
    this->check_locks(lock);
    make_OpenCL_function();
  }//constructor

  //! compution kernel for an iteration (compution=copy, here)
  virtual void kernelGPU(compute::vector<Tdata> &in,compute::vector<Tproc> &out)
  {
    compute::transform(in.begin(), in.end(), out.begin(),
      *vMcPc , this->queue);
  };//kernelGPU

};//CDataProcessorGPU_function

//! complex operation with function macro for GPU process
/**
 *  FMA: val * 2.1 + 123.45
**/
template<typename Tdata,typename Tproc, typename Taccess=unsigned char>
class CDataProcessorGPU_function_macro : public CDataProcessorGPU_vMcPc_check<Tdata,Tproc, Taccess>
{
//OpenCL function for this class
  BOOST_COMPUTE_FUNCTION(Tproc, vMcPc, (Tdata x),
  {
    return x *2.1f + 123.45f;
  });//FUNCTION
public:
  CDataProcessorGPU_function_macro(std::vector<omp_lock_t*> &lock
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
    this->class_name="CDataProcessorGPU_function_macro_vMcPc";
    this->check_locks(lock);
  }//constructor

  //! compution kernel for an iteration (compution=copy, here)
  virtual void kernelGPU(compute::vector<Tdata> &in,compute::vector<Tproc> &out)
  {
    compute::transform(in.begin(), in.end(), out.begin(),
      vMcPc , this->queue);
  };//kernelGPU

};//CDataProcessorGPU_function_macro

//! complex operation with openCL for GPU process
/**
 *  FMA: val * 2.1 + 123.45
 *  \warning: Tdata should be unsigned int as function from source lock type
**/
template<typename Tdata=unsigned int,typename Tproc=unsigned int, typename Taccess=unsigned char>
class CDataProcessorGPU_opencl : public CDataProcessorGPU_vMcPc_check<Tdata,Tproc, Taccess>
{
  compute::program program;
  compute::kernel  kernel;
  bool kernel_loaded;
//OpenCL function for this class
compute::program make_opencl_program(const compute::context& context)
{
  const char source[] = BOOST_COMPUTE_STRINGIZE_SOURCE(
  __kernel void vMcPc(__global const unsigned int*input, int size, __global float*output)
  {
    const int gid = get_global_id(0);
    output[gid]=input[gid]*2.1+123.45;
  }
  );//source
  // create program
  return compute::program::build_with_source(source,context);
}//make_opencl_program

public:
  CDataProcessorGPU_opencl(std::vector<omp_lock_t*> &lock
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
    this->class_name="CDataProcessorGPU_opencl_vMcPc";
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

};//CDataProcessorGPU_opencl

#endif //_DATA_PROCESSOR_GPU_VMCPC_

