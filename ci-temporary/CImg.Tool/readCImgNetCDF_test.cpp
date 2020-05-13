#include <iostream>
#include <vector>
#include <string>

#define cimg_display  0
#include "CImg_NetCDF.h"
#define cimg_debug    2

using namespace std;

#define VERSION "v0.8.3"

int main(int argc,char **argv)
{
///standard command line parameters (e.g. --info, -h, ...)
  cimg_usage(std::string("read NetCDF file in CImg containers (test).\n" \
  " It uses different GNU libraries (see --info option)\n\n" \
  " usage: ./readCImgNetCDF_test -h -I\n" \
  "        ncdump -h CImgNetCDF_cimgListTest.4d-1.nc && ./readCImgNetCDF_test\n" \
  "\n version: "+std::string(VERSION)+"\n compilation date:" \
  ).c_str());//cimg_usage

  #if cimg_display!=0
  const bool show_X=cimg_option("-X",true,NULL);//-X hidden option
  bool show=cimg_option("--show",show_X,"show GUI (or -X option)");show=show_X|show;//same --show or -X option
  #endif
  const bool show_h   =cimg_option("-h",    false,NULL);//-h hidden option
        bool show_help=cimg_option("--help",show_h,"help (or -h option)");show_help=show_h|show_help; //same --help or -h option
  bool show_info=cimg_option("-I",false,NULL);//-I hidden option
  if( cimg_option("--info",show_info,"show compilation options (or -I option)") ) {show_info=true;cimg_library::cimg::info();}//same --info or -I option
  bool show_version=cimg_option("-V",false,NULL);//-v hidden option
  if( cimg_option("--version",show_version,"show version (or -V option)") ) {show_version=true;std::cout<<VERSION<<std::endl<<"CImg_NetCDF."<<CIMG_NETCDF_VERSION<<std::endl;return 0;}//same --version or -v option
  if(show_help) {/*print_help(std::cerr);*/return 0;}
  //}CLI option


  //dims
  int nx;
  int ny;
  int nz;
  int nv;
  int nt;

//nom des dim
 vector<string> dim_names;
  string dim_time="dimt";
/*
  dim_names.push_back("dimx");
  dim_names.push_back("dimy");
  dim_names.push_back("dimz");
  dim_names.push_back("dimv");
*/

//nom de veriables
  vector<string> var_names;
  var_names.push_back("u");
  var_names.push_back("v");
  //var_names.push_back("w");
//nom des unites
  vector<string> unit_names;
  unit_names.push_back("pixel");
  unit_names.push_back("pixel");
  //unit_names.push_back("pixel");

  string var_name="u";
  string unit_name="pixel";

/*
  string fo="CImgNetCDF_cimgListTest.nc";
  CImgNetCDF<float> cimgTest;
  CImg<float> img;

  cout << "CImgNetCDF::loadNetCDFFile(" << fo << ",...) return " << cimgTest.loadNetCDFFile((char*)fo.c_str()) << endl;
  cout << "CImgNetCDF::loadNetCDFDims(" << fo << ",...) return " << cimgTest.loadNetCDFDims(dim_names,&dim_time) << endl;
  cout << "CImgNetCDF::loadNetCDFVar(" << fo << ",...) return "  << cimgTest.loadNetCDFVar(img,var_name,unit_name)  << endl;
  cout << "CImgNetCDF::loadNetCDFData(" << fo << ",...) return " << cimgTest.loadNetCDFData(img)  << endl;
  cout << "img.max()=" << img.max() << endl;
  cout << "CImgNetCDF::loadNetCDFData(" << fo << ",...) return " << cimgTest.loadNetCDFData(img,1) << endl;
  cout << "img.max()=" << img.max() << endl;

  cout << endl;
*/


  //string fo="CImgNetCDF_cimgListTest.nc";
  string fo="CImgNetCDF_cimgListTest.4d-1.nc";



  CImgListNetCDF<float> cimgListTest;
//  CImgList<int> imgList(var_names.size(),nx,ny,nz,nv);
  CImgList<float> imgList;
  cout << "CImgListNetCDF::loadNetCDFFile(" << fo << ",...) return " << cimgListTest.loadNetCDFFile((char*)fo.c_str()) << endl;
  //cout << "CImgListNetCDF::loadNetCDFDims(" << fo << ",...) return " << cimgListTest.loadNetCDFDims(dim_names,&dim_time) << endl;	//names known
  cout << "CImgListNetCDF::loadNetCDFDimNames(" << fo << ",...) return " << cimgListTest.loadNetCDFDimNames(var_names[0],dim_names,&dim_time) << endl;
  

  cout << "The variable names are :"<<endl;
  for(int a = 0 ; a < dim_names.size();a++){
  	cout<< dim_names[a]<<endl;
  }


  cout << "CImgListNetCDF::loadNetCDFVar(" << fo << ",...) return "  << cimgListTest.loadNetCDFVar(imgList,var_names,unit_names)  << endl;
  imgList.print("CImg List");

/*
  for(int t=0;t<1;t++)//cimgListTest.loadDimTime()
  {
    cout << "CImgListNetCDF::loadNetCDFData(" << fo << ",...) return " << cimgListTest.loadNetCDFData(imgList)  << endl;
    cout << "imgList.max()=" << imgList.max() << endl;
  }
*/
  // The file is automatically closed by the destructor. This frees
  // up any internal netCDF resources associated with the file, and
  // flushes any buffers.

  cout << "*** SUCCESS reading example file " << fo << "!" << endl;
  return 0;

}
