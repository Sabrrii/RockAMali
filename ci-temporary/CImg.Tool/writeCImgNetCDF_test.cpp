#include <iostream>
#include <vector>
#include <string>

#define cimg_display  0
#include "CImg_NetCDF.h"
#define cimg_debug    2

using namespace std;

#define VERSION "v0.8.4"

int main(int argc,char **argv)
{
///standard command line parameters (e.g. --info, -h, ...)
  cimg_usage(std::string("write CImg containers in NetCDF file.\n" \
  " It uses different GNU libraries (see --info option)\n\n" \
  " usage: ./writeCImgNetCDF_test -h -I\n" \
  "        ./writeCImgNetCDF_test && ncdump -h CImgNetCDF_CImgTest.nc\n" \
  "\n version: "+std::string(VERSION)+"\n compilation date:" \
  ).c_str());//cimg_usage

// We are writing 5D data
  const int nx=cimg_option("-x",16,"dimx");
  const int ny=cimg_option("-y", 8,"dimy");
  const int nz=cimg_option("-z", 4,"dimz");
  const int nv=cimg_option("-v", 2,"dimv");
  const int nt=cimg_option("-t", 3,"dimt");

  #if cimg_display!=0
  const bool show_X=cimg_option("-X",true,NULL);//-X hidden option
  bool show=cimg_option("--show",show_X,"show GUI (or -X option)");show=show_X|show;//same --show or -X option
  #endif
  const bool show_h   =cimg_option("-h",    false,NULL);//-h hidden option
        bool show_help=cimg_option("--help",show_h,"help (or -h option)");show_help=show_h|show_help; //same --help or -h option
  bool show_info=cimg_option("-I",false,NULL);//-I hidden option
  if( cimg_option("--info",show_info,"show compilation options (or -I option)") ) {show_info=true;cimg_library::cimg::info();}//same --info or -I option
  bool show_version=cimg_option("-V",false,NULL);//-V hidden option
  if( cimg_option("--version",show_version,"show version (or -v option)") ) {show_version=true;std::cout<<VERSION<<std::endl<<"CImg_NetCDF."<<CIMG_NETCDF_VERSION<<std::endl;return 0;}//same --version or -v option
  if(show_help) {/*print_help(std::cerr);*/return 0;}
  //}CLI option

//dimension names
  vector<string> dim_names;
  string dim_time="dimF";
  dim_names.push_back("dimS");
/*
  dim_names.push_back("dimy");
  dim_names.push_back("dimz");
  dim_names.push_back("dimv");
*/

//variable names (and its unit)
///single
  string var_name="signal";
  string unit_name="none";
///list
/**/
  vector<string> var_names;
  var_names.push_back("u");
  var_names.push_back("v");
/*
  var_names.push_back("w");
*/
  vector<string> unit_names;
  unit_names.push_back("pixel");
  unit_names.push_back("pixel");
/*
  unit_names.push_back("pixel");
*/

/*inheritance test/
  string fo="CImgNetCDF_test.nc";
  CImgNetCDF_test<float> test;
  cout << "CImgNetCDF_test::saveNetCDFFile(" << fo << ",...) return " << test.saveNetCDFFile((char*)fo.c_str(),dim_names,dim_time) << endl;
  test.print();
  cout << "CImgNetCDF_test::addNetCDFVar(" << fo << ",...) return " << test.addNetCDFVar(var_name,unit_name) << endl;
  cout << "CImgNetCDF_test::addNetCDFData" << fo << ",...) return " << test.addNetCDFData() << endl;
  cout << "CImgNetCDF_test::addNetCDFData" << fo << ",...) return " << test.addNetCDFData() << endl;
  cout << endl;
/**/

/*CImg test*/
  string/**/ fo="CImgNetCDF_CImgTest.nc";
  typedef int Tdata;//char, int, float, double, unsigned char (byte) ok
  CImgNetCDF<Tdata> cimgTest;
  CImg<Tdata> img(nx,ny,nz,nv);
  cout << "CImgNetCDF::saveNetCDFFile(" << fo << ",...) return " << cimgTest.saveNetCDFFile((char*)fo.c_str()) << endl;
  cout << "CImgNetCDF::addNetCDFDims(" << fo << ",...) return " << cimgTest.addNetCDFDims(img,dim_names,dim_time) << endl;
  cout << "CImgNetCDF::addNetCDFVar(" << fo << ",...) return " << cimgTest.addNetCDFVar(img,var_name,unit_name) << endl;
  //add attributes
  if (!(cimgTest.pNCvar->add_att("long_name","fake test signal, e.g. data (lib/class CImgNetCDF)"))) std::cerr<<"error: while adding attribute long name (NC_ERROR)."<<std::endl;
  for(int t=0;t<nt;++t)
  {
    cimg_forXYZC(img,x,y,z,v) img(x,y,z,v)=t*x*y*z*v+x+y+z+v+t;
    cout << "CImgNetCDF::addNetCDFData(" << fo << ",...) return " << cimgTest.addNetCDFData(img) << endl;
  }
  //add global attributes
  cimgTest.pNCFile->add_att("library","CImg_NetCDF");
  cimgTest.pNCFile->add_att("library_version",CIMG_NETCDF_VERSION);
  cout << endl;
/**/

/*CImgList test/
  string/**/ fo="CImgNetCDF_CImgListTest.nc";
  CImgListNetCDF<float> cimgListTest;
  CImgList<float> imgList(var_names.size(),nx,ny,nz,nv);
////file
  cout << "CImgListNetCDF::saveNetCDFFile(" << fo << ",...) return " 	<< cimgListTest.saveNetCDFFile((char*)fo.c_str()) << endl;
////dim
  cout << "CImgListNetCDF::addNetCDFDims(" << fo << ",...) return " 	<< cimgListTest.addNetCDFDims(imgList,dim_names,dim_time) << endl;
////var
  cout << "CImgListNetCDF::addNetCDFVar(" << fo << ",...) return " 	<< cimgListTest.addNetCDFVar(imgList,var_names,unit_names) << endl;
////data
  for(int t=0;t<nt;++t)
  {
    cimglist_for(imgList,l) cimg_forXYZC(imgList(l),x,y,z,v) imgList(l)(x,y,z,v)=t*x*y*z*v*l+x+y+z+v+t+l;
    cout << "CImgListNetCDF::addNetCDFData(" << fo << ",...) return " 	<< cimgListTest.addNetCDFData(imgList) << endl;
  }
  imgList.print("CImg List");
/**/

/*4 dimensions: CImgList<T> and CImg<t>/
  string /**/fo="CImgNetCDF_CImgListnCImgTest.4D+1.nc";
/* */
  CImgListNetCDF<float> cimgListTest4D;
  CImgList<float> imgList4D(var_names.size(),nx,ny,nz,nv);
  CImgNetCDF<int> cimgTest4D;
  CImg<int> img4D(nx,ny,nz,nv);
////file
  cout << "CImgListNetCDF::saveNetCDFFile(" << fo << ",...) return " 	<< cimgListTest4D.saveNetCDFFile((char*)fo.c_str()) << endl;
  cout << "CImgNetCDF::setNetCDFFile(" << fo << ",...) return " 	<< cimgTest4D.setNetCDFFile(cimgListTest4D.pNCFile) << endl;//share file
////dim
  cout << "CImgListNetCDF::addNetCDFDims(" << fo << ",...) return " 	<< cimgListTest4D.addNetCDFDims(imgList4D,dim_names,dim_time) << endl;
  cout << "CImgNetCDF::setNetCDFDims(" << fo << ",...) return " 	<< cimgTest4D.setNetCDFDims(cimgListTest4D.vpNCDim,cimgListTest4D.pNCDimt) << endl;//share dims
////var
  cout << "CImgListNetCDF::addNetCDFVar(" << fo << ",...) return " 	<< cimgListTest4D.addNetCDFVar(imgList4D,var_names,unit_names) << endl;
  cout << "CImgNetCDF::addNetCDFVar(" << fo << ",...) return " 	<< cimgTest4D.addNetCDFVar(img4D,var_name,unit_name) << endl;
////data
//////t=0
  cimglist_for(imgList4D,l) cimg_forXYZC(imgList4D(l),x,y,z,v) imgList4D(l)(x,y,z,v)=x*y*z*v*l;
  cimg_forXYZC(img4D,x,y,z,v) img4D(x,y,z,v)=1;

  cout << "CImgListNetCDF::addNetCDFData(" << fo << ",...) return " 	<< cimgListTest4D.addNetCDFData(imgList4D) << endl;
  cout << "CImgNetCDF::addNetCDFData(" << fo << ",...) return " 	<< cimgTest4D.addNetCDFData(img4D,cimgListTest4D.loadDimTime()-1) << endl;

//////t=1
  cimglist_for(imgList4D,l) cimg_forXYZC(imgList4D(l),x,y,z,v) imgList4D(l)(x,y,z,v)=(x*y*z*v+1)*(l+1);
  cimg_forXYZC(img4D,x,y,z,v) img4D(x,y,z,v)=0;

  cout << "CImgListNetCDF::addNetCDFData" << fo << ",...) return " 	<< cimgListTest4D.addNetCDFData(imgList4D) << endl;
  cout << "CImgNetCDF::addNetCDFData" << fo << ",...) return " 	<< cimgTest4D.addNetCDFData(img4D,cimgListTest4D.loadDimTime()-1) << endl;

  cout << endl;



//dimension 3
/*
  fo="CImgNetCDF_CImgListnCImgTest.3D+1.nc";
  CImgListNetCDF<int> cimgListTest;
  CImgList<int> imgList(var_names.size(),nx,ny,nz);
  cout << "CImgListNetCDF::saveNetCDFFile(" << fo << ",...) return " 	<< cimgListTest.saveNetCDFFile((char*)fo.c_str()) << endl;
  cout << "CImgListNetCDF::addNetCDFDims(" << fo << ",...) return " 	<< cimgListTest.addNetCDFDims(imgList,dim_names,dim_time) << endl;
  cout << "CImgListNetCDF::addNetCDFVar(" << fo << ",...) return " 	<< cimgListTest.addNetCDFVar(imgList,var_names,unit_names) << endl;

  cimglist_for(imgList,l) cimg_forXYZ(imgList(l),x,y,z) imgList(l)(x,y,z)=x*y*z*l;

  cout << "CImgListNetCDF::addNetCDFData" << fo << ",...) return " 	<< cimgListTest.addNetCDFData(imgList) << endl;

  cimglist_for(imgList,l) cimg_forXYZ(imgList(l),x,y,z) imgList(l)(x,y,z)=(x*y*z+1)*(l+1);

  cout << "CImgListNetCDF::addNetCDFData" << fo << ",...) return " 	<< cimgListTest.addNetCDFData(imgList) << endl;
  cout << endl;
*/


/*
  fo="CImgNetCDF_test_CImg.nc";CImgNetCDF_cimgListTest.nc
  CImgNetCDF_test_CImg<double> image(nx,ny,nz,nv);
  cout << "CImgNetCDF_test_CImg::saveNetCDFFile(" << fo << ",...) return " << image.saveNetCDFFile((char*)fo.c_str(),dim_names,dim_time) << endl;
  image.print();
  cout << "CImgNetCDF_test_CImg::addNetCDFVar(" << fo << ",...) return " << image.addNetCDFVar(var_name,unit_name) << endl;
  for(int t=0;t<nt;t++)
  {
    int i=t;cimg_forXYZC(image,x,y,z,v) image(x,y,z,v)=i++;
    cout << "CImgNetCDF_test_CImg::addNetCDFData" << fo << ",...) return " << image.addNetCDFData() << endl;
  }
*/
/*
  fo="CImgNetCDF_CImg.nc";
  var_name="CImg";
  unit_name="pixel";
 CImg<float> image(nx,ny,nz,nv);
  cout << "CImg::saveNetCDFFile(" << fo << ",...) return " << image.saveNetCDFFile((char*)fo.c_str(),dim_names,dim_time) << endl;
  image.print();
  cout << "CImg::addNetCDFVar(" << fo << ",...) return " << image.addNetCDFVar(var_name,unit_name) << endl;
  for(int t=0;t<nt;t++)
  {
    int i=t;cimg_forXYZC(image,x,y,z,v) image(x,y,z,v)=i++;
    cout << "CImg::addNetCDFData" << fo << ",...) return " << image.addNetCDFData() << endl;
  }
*/
   // The file is automatically closed by the destructor. This frees
   // up any internal netCDF resources associated with the file, and
   // flushes any buffers.
   cout << "*** SUCCESS writing example file " << fo << "!" << endl;
   return 0;
}
