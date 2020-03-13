#ifndef _DATA_RECEIVE_
#define _DATA_RECEIVE_

//C base
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#include "CDataBuffer.hpp"

template<typename Tdata, typename Taccess=unsigned char>
class CDataReceive : public CDataBuffer<Tdata, Taccess>
{
public:
  //rate related
  unsigned long t0,t1;
  //UDP related
  int udpSocket,nBytes;
  //! content buffer (as char)
  CImg<unsigned char> buffer;
  struct sockaddr_in receiverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;

  bool do_check;unsigned int check_error;bool do_check_exit;

  CDataReceive(std::vector<omp_lock_t*> &lock, std::string src_ip, unsigned short port, int buf_size, bool do_check=false, bool do_check_exit=false) : CDataBuffer<Tdata, Taccess>(lock)
   , s(new udp_server<Tdata>(*io_service, port, buf_size))
   , io_service(io_service)
   , do_check(do_check), do_check_exit(do_check_exit)
  {
//    this->debug=true;
    this->class_name="CDataReceiver";
    buffer.assign(buf_size);
    this->check_locks(lock);
    check_error=0;
  }

  //! copy the data in a vector in a CImg
  void copy2cimg(std::vector<Tdata> *vec,CImg<Tdata>&image)
  {
    for(unsigned int i=0; i < vec->size(); ++i)
    {
      image[i]=static_cast<Tdata>((*vec)[i]);
    }
  }//copy2cimg

  virtual void iteration(CImg<unsigned char> &access,CImgList<Tdata> &images, int n, int i)
  {
    if(this->debug)
    {
      this->lprint.print("",false);
      printf("4 B%02d #%04d: ",n,i);fflush(stdout);
      access.print("access",false);fflush(stderr);
      this->lprint.unset_lock();
    }

    //wait lock
    unsigned int c=0;
    this->laccess.wait_for_status(access[n],this->STATUS_FREE,this->STATE_RECEIVING, c);//free, receiving

    //! \todo [low] get UDP buffer

    //copy buffer to circular buffer
    copy2cimg(&rec_buf,images[n]);

    //check
    if(do_check)
    {
      //if(images[n]==(i+1)) //slow check, but entire frame
      //if(images[n](0)==(i+1)) //fast check
      if(images[n](0)==i) //fast random check
      NULL; else {++check_error;std::cout<<"receive error: bad check (i.e. test failed) on iteration #"<<i<<" (value="<<images[n](0)<<")."<<std::endl<<"test: fail."<<std::flush;if(do_check_exit) exit(4);}
    }

    this->laccess.set_status(access[n],this->STATE_RECEIVING,this->STATUS_RECEIVED, this->class_name[5],i,n,c);//receiving, received
  }//iteration

  virtual void show_checking()
  {
    if(do_check)
    {
    if(check_error>0) std::cout<<"test: fail ("<<check_error<<" errors)."<<std::endl;
    else std::cout<<"test: pass."<<std::endl;
    }//do_check
  }//show_checking
};//CDataReceive

#endif //_DATA_RECEIVE_

