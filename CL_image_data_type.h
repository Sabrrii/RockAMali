#ifndef CL_IMAGE_DATA_TYPE_INFORMATION
#define CL_IMAGE_DATA_TYPE_INFORMATION

#define CL_IMAGE_DATA_TYPE_INFO_VERSION "v0.1.4e"

//! Information on OpenCL type of image data
/**
 * Information on OpenCL type of image data, implemented types are the following:
 *      \c compute::uint4_,         \c compute::float4_
 * \note This list may not be updated, but have a look in \e Class \e Hierarchy of documentation
**/

template<typename T> struct CLTypeInfo {
//! Identifier - it selves (i.e. \c <int>value   of \c CL_ from \c opencl.h) for OpenCL image data
  static         int clId()  {static const int         i= CL_UNSIGNED_INT8;  return i;}
//! Identifier as a string (i.e. \c <char*>value of \c CL_ from \c opencl.h) for OpenCL image data
  static const char* clStr() {static const char *const s="CL_UNSIGNED_INT8"; return s;}
};
//! Information on OpenCL image data type of a \c char data
template<> struct CLTypeInfo<compute::uint4_> {
  static         int clId()  {static const int         i= CL_UNSIGNED_INT32;  return i;}
  static const char* clStr() {static const char *const s="CL_UNSIGNED_INT32"; return s;}
};
//! Information on OpenCL image data type of a \c short data
template<> struct CLTypeInfo<compute::float4_> {
  static       int   clId()  {static const short       i= CL_FLOAT;  return i;}
  static const char* clStr() {static const char *const s="CL_FLOAT"; return s;}
};

//! Information on type as a string of any \c CL
char *CLTypeStr(int clId)
{
  char *s;
  switch(clId)
  {
    case CL_UNSIGNED_INT8: s=(char *)(CLTypeInfo<compute::uint4_>::clStr());  break;
    case CL_FLOAT:         s=(char *)(CLTypeInfo<compute::float4_>::clStr()); break;
    default: return NULL;
  }
  return s;
}//CLTypeStr

#endif/*CL_IMAGE_DATA_TYPE_INFORMATION*/

