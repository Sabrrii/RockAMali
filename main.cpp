#include "CImg.h"
#include <iostream>
#include <string>

//! \todo [medium] OpenMP
//! \todo [low] gen+store

using namespace cimg_library;

#define VERSION "v0.0.2"

#define S 0 //sample

int main(int argc,char **argv)
{
  ///command arguments, i.e. CLI option
  cimg_usage(std::string("generate and store data.\n" \
  " It uses different GNU libraries (see --info option)\n\n" \
  " usage: ./store -h -I\n" \
  "        ./store -s 1024 -n 123 -X true -o sample.cimg && ls sample_000???.cimg\n" \
  "\n version: "+std::string(VERSION)+"\n compilation date:" \
  ).c_str());//cimg_usage

  const char* imagefilename = cimg_option("-o","sample.cimg","output file name");
  const int width=cimg_option("-s",1024,  "size   of vector");
  const int count=cimg_option("-n",123,   "number of vector");
  const int nbuffer=cimg_option("-b",12,  "size   of vector buffer (total size is b*s)");
  ///standard options
  #if cimg_display!=0
  const bool show_X=cimg_option("-X",true,NULL);//-X hidden option
  bool show=cimg_option("--show",show_X,"show GUI (or -X option)");show=show_X|show;//same --show or -X option
  #endif
  const bool show_h   =cimg_option("-h",    false,NULL);//-h hidden option
        bool show_help=cimg_option("--help",show_h,"help (or -h option)");show_help=show_h|show_help; //same --help or -h option
  bool show_info=cimg_option("-I",false,NULL);//-I hidden option
  if( cimg_option("--info",show_info,"show compilation options (or -I option)") ) {show_info=true;cimg_library::cimg::info();}//same --info or -I option
  bool show_version=cimg_option("-v",false,NULL);//-v hidden option
  if( cimg_option("--version",show_version,"show version (or -v option)") ) {show_version=true;std::cout<<VERSION<<std::endl;return 0;}//same --version or -v option
  if(show_help) {/*print_help(std::cerr);*/return 0;}
  //}CLI option

  //! circular buffer
  CImgList<unsigned char> images(nbuffer,width,1,1,1);
  for(int n=0,i=0;i<count;++i,++n)
  {
    //fill image
    images[n].fill(i);
    //save image
    CImg<char> nfilename(1024);
    cimg::number_filename(imagefilename,i,6,nfilename);
    images[n].save_cimg(nfilename);
    //circular buffer
    if(n==nbuffer-1) n=-1;
  }//vector loop
//  images.print("CImgList");
  return 0;
}//main
