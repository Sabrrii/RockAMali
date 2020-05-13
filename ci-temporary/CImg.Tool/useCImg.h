#ifndef USECIMG
#define USECIMG

//using CImg library
///graphical user interface: set to 0 to force to none, one may set it on compiler command line option (e.g. g++ ... -Dcimg_display_type=0 ...)
#define cimg_display_type  0
///debug message: set to 0 to force to none, 1, 2 ... 5 for more debugging messages
//#define cimg_debug         1

///add feature to CImg library using PlugIn header
////libPNG image support
//#include "include/png.h"
////libTIFF image support
//#include "include/tiffio.h"

////PlugIns
//#define cimg_plugin           "../CImg.PlugIn/PlugIn.CImg.h"
//#define cimgnamespace_plugin  "../CImg.PlugIn/PlugIn.CImgLibrary.h"
///CImg library
#include "../CImg/CImg.h"
using namespace cimg_library;
/// The lines below are not necessary in your own code, it simply allows 
/// the source compilation with compilers that do not respect the C++ standart.
#if ( defined(_MSC_VER) && _MSC_VER<=1200 ) || defined(__DMC__)
#define std
#endif

#endif/*USECIMG*/
