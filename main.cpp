#include "CImg.h"
#include <iostream>
#include <string>

//OpenMP
#include <omp.h>
#include <vector>

//! \todo [high] class: CBaseOMPLock, +print, +progress, +buffer, +run: gen,store

using namespace cimg_library;

#define VERSION "v0.0.8e"

#define S 0 //sample

class CBaseOMPLock
{
public:
  std::string class_name;
  int id;
  int tn;

  CBaseOMPLock(){class_name="CBaseOMPLock";id=omp_get_thread_num();tn=omp_get_num_threads();}
  CBaseOMPLock(std::vector<omp_lock_t*> &lock){CBaseOMPLock();}
  virtual void unset_lock(){}
  virtual void print(char* message, bool unset=true){printf("class=%s\n",class_name.c_str());printf(message);}
};//CBaseOMPLock

class CPrintOMPLock: public CBaseOMPLock
{
public:
  omp_lock_t *p_print_lock;
  CPrintOMPLock(std::vector<omp_lock_t*> &lock):CBaseOMPLock(lock){class_name="CPrintOMPLock";if(lock.size()>0) p_print_lock=lock[0];else{printf("code error: locks should have at least 1 lock for %s class.",class_name.c_str());exit(99);}}
//  CPrintOMPLock(omp_lock_t* lock){std::vector<omp_lock_t*> locks;locks.push_back(lock);CPrintOMPLock(locks);}
  virtual void unset_lock(){omp_unset_lock(p_print_lock);}
  virtual void print(char* message, bool unset=true)
  {//locked section
    omp_set_lock(p_print_lock);
    printf("class=%s\n",class_name.c_str());
    printf("t%d/%d %s",id,tn,message);
    fflush(stdout);
    if(unset) omp_unset_lock(p_print_lock);
  }//print
};//CPrintOMPLock


class CAccessOMPLock: public CBaseOMPLock
{
public:
  omp_lock_t *p_access_lock;
  CAccessOMPLock(std::vector<omp_lock_t*> &lock):CBaseOMPLock(lock){class_name="CAccessOMPLock";if(lock.size()>0) p_access_lock=lock[0];else{printf("code error: locks should have at least 1 lock for %s class.",class_name.c_str());exit(99);}}
  CAccessOMPLock(omp_lock_t* lock){std::vector<omp_lock_t*> locks;locks.push_back(lock);CAccessOMPLock((std::vector<omp_lock_t*>)locks);}
  virtual void unset_lock(){omp_unset_lock(p_access_lock);}
  virtual void wait_for_status(unsigned char &what, int status, int new_status, unsigned int &c)
  {
    unsigned char a=99;
    do
    {//waiting for status
      //locked section
      {
        omp_set_lock(p_access_lock);
        a=what;
        if(a==status) what=new_status;
        omp_unset_lock(p_access_lock);
      }//lock
      ++c;
    }while(a!=status);//waiting for free
  }//wait_for_status


  virtual void set_status(unsigned char &what, int old_status, int status, /*info:*/ unsigned int i, unsigned int n, unsigned int c)
  {//locked section
    omp_set_lock(p_access_lock);
    //debug
    if(what!=old_status)/*filling*/ {printf("error: code error, acces should be 0x%x i.e. Filling for buffer#%d (presently is is 0x%x)",old_status,n,what);omp_unset_lock(p_access_lock);exit(99);}
    what=status;//filled

    //debug misc.
    printf("G%d/%d 4 B%02d #%04d wait=%d\n",id,tn,n,i,c);fflush(stdout);

    omp_unset_lock(p_access_lock);
  }//set_status


};//CAccessOMPLock

class CDataGenerator
{
public:
  std::string class_name;
  CPrintOMPLock  lprint;
  CAccessOMPLock laccess;

  CDataGenerator(std::vector<omp_lock_t*> &lock) : lprint(lock/*[0]*/), laccess(lock/*[1]*/) {class_name="CDataGenerator";if(lock.size()<2) {printf("code error: locks should have at least 2 lock for %s class.",class_name.c_str());exit(99);}}
  virtual void iteration(CImg<unsigned char> &access,CImgList<unsigned int> &images, int n, int i)
  {
    //wait lock
    unsigned int c=0;
    laccess.wait_for_status(access[n],0x0,0xF, c);//free,filling

    //fill image
    images[n].fill(i);

    //set filled
    laccess.set_status(access[n],0xF,0x1, i,n,c);//filling,filled
  }
};//CDataGenerator

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
  const int width=cimg_option("-s",1024, "size   of vector");
  const int count=cimg_option("-n",123,  "number of vector");
  const int nbuffer=cimg_option("-b",12, "size   of vector buffer (total size is b*s*4 Bytes)");
  const int threadCount=cimg_option("-c",0,"thread count");
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

  //OpenMP
  if(threadCount>0)
  {//user number of thread
    omp_set_dynamic(0);
    omp_set_num_threads(threadCount);
  }//user

  //OpenMP locks
  omp_lock_t print_lock;omp_init_lock(&print_lock);
  std::vector<omp_lock_t*> locks;locks.push_back(&print_lock);
  //OpenMP print
  CPrintOMPLock pr(locks);

  //! circular buffer
  CImgList<unsigned int> images(nbuffer,width,1,1,1);
  images[0].fill(0);
  images[0].print("image",false);
  //access locking
  omp_lock_t lck;omp_init_lock(&lck);
  locks.clear();locks.push_back(&lck);
  CAccessOMPLock lacces(locks);

  //! access and status of buffer
  CImg<unsigned char> access(nbuffer,1,1,1);
  access.fill(0);//free
  access.print("access (free state)",false);fflush(stderr);

  //! generate data
  locks.clear();locks.push_back(&print_lock);locks.push_back(&lck);
  CDataGenerator generate(locks);

  #pragma omp parallel shared(pr, access,lck, images)
  {
  int id=omp_get_thread_num(),tn=omp_get_num_threads();
  #pragma omp single
  {
  if(tn<2) {printf("error: run error, this process need at least 2 threads (presently only %d available)\n",tn);exit(2);}
  else {printf("info: running %d threads\n",tn);fflush(stdout);}
  }//single
  for(int n=0,i=0;i<count;++i,++n)
  {
/**/
  if(id<2)//2 for 2 threads
  {//locked section
  pr.print(" message ",false);
  printf("|t%d/%d|",id,tn);//temp
  printf("4 B%02d #%04d: ",n,i);fflush(stdout);
  access.print("access",false);fflush(stderr);
  pr.unset_lock();
  }//lock
/**/
    switch(id)
    {
      case 0:
      {//generate
        generate.iteration(access,images, n,i);
        break;
      }//generate
      case 1:
      {//store
        //wait lock
        unsigned int c=0;
        lacces.wait_for_status(access[n],0x1,0x5, c);//filled,storing

        //save image
        CImg<char> nfilename(1024);
        cimg::number_filename(imagefilename,i,6,nfilename);
        images[n].save_cimg(nfilename);

        //set filled
        lacces.set_status(access[n],0x5,0x0, i,n,c);//storing,free
        break;
      }//store
    }//switch(id)
    //circular buffer
    if(n==nbuffer-1) n=-1;
  }//vector loop
  }//parallel section

  access.print("access (free state)",false);fflush(stderr);
  images.print("CImgList",false);
  return 0;
}//main

