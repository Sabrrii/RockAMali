#ifndef _THREAD_LOCK_
#define _THREAD_LOCK_

//C++ base
#include <iostream>
#include <string>

//OpenMP
#include <omp.h>

//! base lock (with OpenMP)
class CBaseOMPLock
{
public:
  std::string class_name;
  int id;
  int tn;
  bool debug;

  CBaseOMPLock(omp_lock_t* lock){debug=false;class_name="CBaseOMPLock";id=omp_get_thread_num();tn=omp_get_num_threads(); if(debug) printf("t%d/%d,class=%s\n",id,tn,class_name.c_str());/*warning*/if(0) printf("%p",(void*)lock);}
  virtual void unset_lock(){}
  virtual void print(const char* message, bool unset=true){printf("t%d/%d ",id,tn); if(debug) printf(",class=%s\n",class_name.c_str());printf(message /*warning*/,unset);}
};//CBaseOMPLock

//! print lock (with OpenMP)
class CPrintOMPLock: public CBaseOMPLock
{
public:
  omp_lock_t *p_print_lock;
  CPrintOMPLock(omp_lock_t* lock):CBaseOMPLock(lock){class_name="CPrintOMPLock"; p_print_lock=lock;}
  virtual void unset_lock(){omp_unset_lock(p_print_lock);}
  virtual void print(const char* message, bool unset=true)
  {//locked section
    omp_set_lock(p_print_lock);
    if(debug) printf("class=%s\n",class_name.c_str());
    printf("t%d/%d %s",id,tn,message);
    fflush(stdout);
    if(unset) omp_unset_lock(p_print_lock);
  }//print
};//CPrintOMPLock

//! access lock (with OpenMP)
class CAccessOMPLock: public CBaseOMPLock
{
public:
  omp_lock_t *p_access_lock;
  CAccessOMPLock(omp_lock_t* lock):CBaseOMPLock(lock){debug=true;class_name="CAccessOMPLock"; p_access_lock=lock;}
  virtual void unset_lock(){omp_unset_lock(p_access_lock);}
  virtual void wait_for_status(unsigned char &what, const int status, const int new_status, unsigned int &c, unsigned int sleep=1234)
  {
    unsigned char a=99;
    bool do_sleep=true;
    do
    {//waiting for status
      //locked section
      {
        omp_set_lock(p_access_lock);
        a=what;
        if(a==status) {what=new_status;do_sleep=false;}
        omp_unset_lock(p_access_lock);
      }//lock
      ++c;
      if(do_sleep) usleep(sleep);
    }while(a!=status);//waiting for status
  }//wait_for_status


  virtual void set_status(unsigned char &what, int old_status, int status, /*info:*/ char me, unsigned int i, unsigned int n, unsigned int c, int lsleep=-1)
  {//locked section
    omp_set_lock(p_access_lock);
    //debug
    if(what!=old_status)/*filling*/ {printf("error: code error, acces should be 0x%x i.e. Filling for buffer#%d (presently is is 0x%x)",old_status,n,what);omp_unset_lock(p_access_lock);exit(99);}
    what=status;//filled

    //! \todo [high] need print lock and move after "omp_unset_lock(p_access_lock);"
    if(debug)
    {
      printf("%c%d/%d 4 B%02d #%04d wait=%d",me,id,tn,n,i,c);
      if(lsleep>0)
      {
        unsigned int gsleep=lsleep*c;
        std::string lu("us"),gu("us");
        if(lsleep>999) {lsleep/=1000;lu="ms";}
        if(gsleep>999) {gsleep/=1000;gu="ms";}
        printf(", local sleep=%d%s, global sleep=%d%s",lsleep,lu.c_str(),gsleep,gu.c_str());
      }
      std::cout<<std::endl<<std::flush;
    }//debug

    omp_unset_lock(p_access_lock);

  }//set_status


};//CAccessOMPLock

#endif //_THREAD_LOCK_

