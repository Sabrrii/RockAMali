#ifndef NETCDF_INFORMATION
#define NETCDF_INFORMATION

#include <netcdfcpp.h>

#define NETCDF_TYPE_INFO_VERSION "v0.1.4"

#define NC_ERROR   -2
#define CODE_ERROR -3
#define DIM_ERROR  -4

//! Information on NetCDF type of data
/**
 * Information on NetCDF type of data, implemented types are the following:
 *      \c char,         \c int,         \c float
 * \see NcTypeInfo<char> NcTypeInfo<int> NcTypeInfo<float>
 * \note This list may not be updated, but have a look in \e Class \e Hierarchy of documentation
**/

template<typename T> struct NcTypeInfo {
//! Identifier - it selves (i.e. \c <int>value of \c NcType from \c ncvalues.h) on NetCDF type of data
  static         int ncId()  {static const         int i= ncNoType;  return i;}
//! Identifier as a string (i.e. \c <char*>value of \c NcType from \c ncvalues.h) on NetCDF type of data
  static const char* ncStr() {static const char *const s="ncNoType"; return s;}
};
//! Information on NetCDF type of a \c char data
template<> struct NcTypeInfo<char> {
  static         int ncId()  {static const         int i= ncChar;  return i;}
  static const char* ncStr() {static const char *const s="ncChar"; return s;}
};
//! Information on NetCDF type of a \c short data
template<> struct NcTypeInfo<short> {
  static         int ncId()  {static const       short i= ncChar;  return i;}
  static const char* ncStr() {static const char *const s="ncShort"; return s;}
};
//! Information on NetCDF type of an \c int data
template<> struct NcTypeInfo<int> {
  static         int ncId()  {static const         int i= ncInt;  return i;}
  static const char* ncStr() {static const char *const s="ncInt"; return s;}
};
//! Information on NetCDF type of a \c float data
template<> struct NcTypeInfo<float> {
  static         int ncId()  {static const         int i= ncFloat;  return i;}
  static const char* ncStr() {static const char *const s="ncFloat"; return s;}
};
//! Information on NetCDF type of a \c double data
template<> struct NcTypeInfo<double> {
  static       int   ncId()  {static const         int i= ncDouble;  return i;}
  static const char* ncStr() {static const char *const s="ncDouble"; return s;}
};
//! Information on NetCDF type of a \c bool data
template<> struct NcTypeInfo<bool> {
  static         int ncId()  {static const         int i= ncByte;                    return i;}
  static const char* ncStr() {static const char *const s="ncByte (used as boolean)"; return s;}
};
//! Information on NetCDF type of a \c bool data
template<> struct NcTypeInfo<ncbyte> {
  static         int ncId()  {static const         int i= ncByte;                    return i;}
  static const char* ncStr() {static const char *const s="ncByte (used as boolean)"; return s;}
};
//! Information on NetCDF type as a string of any \c NcType
char *NcTypeStr(int ncId)
{
  char *s;
  switch(ncId)
  {
    case ncChar:  s=(char *)(NcTypeInfo<char>::ncStr());  break;
    case ncShort: s=(char *)(NcTypeInfo<short>::ncStr()); break;
    case ncInt:   s=(char *)(NcTypeInfo<int>::ncStr());   break;
    case ncFloat: s=(char *)(NcTypeInfo<float>::ncStr()); break;
    case ncDouble:s=(char *)(NcTypeInfo<double>::ncStr());break;
    case ncByte:  s=(char *)(NcTypeInfo<bool>::ncStr());  break;
    default: return NULL;
  }
  return s;
}


#endif/*NETCDF_INFORMATION*/

