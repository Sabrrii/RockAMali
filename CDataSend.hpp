#ifndef _DATA_SEND_
#define _DATA_SEND_

#include <boost/asio/ip/udp.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "UDP/high_res_clock.hpp"

using boost::asio::ip::udp;
using boost::posix_time::ptime;
using boost::posix_time::microsec_clock;

#include "CDataAccess.hpp"

template<typename Tdata, typename Taccess=unsigned char>
class CDataSend : public CDataBuffer<Tdata, Taccess>
{
public:

  boost::asio::io_service io_service;
  udp::socket socket;
  udp::endpoint target;
  boost::uint64_t wait;
  std::vector<Tdata> write_buf;

  CDataSend(std::vector<omp_lock_t*> &lock, std::string ip, unsigned short port, boost::uint64_t wait) : CDataBuffer<Tdata, Taccess>(lock)
    , socket(io_service, udp::endpoint(udp::v4(), 0))
    , target(boost::asio::ip::address::from_string(ip), port)
    , wait(wait)
  {
//    this->debug=true;
    this->class_name="CDataSender";
    udp::socket::non_blocking_io nbio(true);
    socket.io_control(nbio);
    target.port(port);
    this->check_locks(lock);
  }//constructor

  //! copy the data in a CImg in a vector
  void copy2vector(CImg<Tdata> img)
  {
    write_buf.clear();
    for(int i=0; i<img.width(); ++i)
    {
      write_buf.push_back(static_cast<Tdata>(img[i]));
    }
  }//copy2vector

  virtual void iteration(CImg<Taccess> &access,CImgList<Tdata> &images, int n, int i)
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
    this->laccess.wait_for_status(access[n],this->STATUS_FILLED,this->STATE_SENDING, c);//filled, sending

    //! \todo [high] . move copy2vector to iteration, consequently suppress run that should inherit from CDataBuffer
    copy2vector(images[n]);

    //send data by udp
    boost::system::error_code ec;
    socket.send_to(boost::asio::buffer(write_buf), target, 0, ec);
    boost::uint64_t time_hr = high_res_clock();
    while ((high_res_clock()-time_hr)<wait) {}

    //set free
    this->laccess.set_status(access[n],this->STATE_SENDING,this->STATUS_FREE, this->class_name[5],i,n,c);//sent, now free

  }//iteration

};//CDataSend

#endif //_DATA_SEND_

