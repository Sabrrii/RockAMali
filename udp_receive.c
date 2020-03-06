#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

// UDP point to point draft test (UDP frame from ml507 at 752Mbps, i.e. 80MB/s)

//! \todo [draft] argp for loop
//! \todo [draft] . check increment
//! \todo [draft] count (and show) increment error (# and step)
//! \todo [draft] swap endianness depending of arch.

// UDP point to point test

//! \todo add CImg for option and buffer
//! \todo tests: ml507, RockAMali, numexo2

int main()
{
  int udpSocket, nBytes;
  char buffer[2048];
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size, client_addr_size;

  /*Create UDP socket*/
  udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

  /*Configure settings in address struct*/
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(20485);
  serverAddr.sin_addr.s_addr = inet_addr("10.10.17.202");
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  /*Bind socket with address struct*/
  bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

  /*Initialize size variable to be used later on*/
  addr_size = sizeof serverStorage;

  //index
  unsigned int index=0;
  unsigned int prev_index=0;
  //index increment
  long inc=0;
//  while(1)
  for(unsigned int i=0;i<256;++i)
  {
    /* Try to receive any incoming UDP datagram. Address and port of 
      requesting client will be stored on serverStorage variable */
//    nBytes = recvfrom(udpSocket,buffer,2048,0,(struct sockaddr *)&serverStorage, &addr_size);
//! increment frame index (on first byte only), and simulate a frame drop at loop index 123 (note: looping over size of byte yield to -255 step)
buffer[0]=0x12+((i<123)?i:i+12);
buffer[1]=0x34;
buffer[2]=0x56;
buffer[3]=0x78;
    //get frame index as first uint32 of buffer content
    {const unsigned int *b=(unsigned int *)buffer; index=*b;}//! \todo endianess
    //check increment
    inc=(long)index-(long)prev_index;
    if(inc!=1)
    {//frame drop
      //print loop index
      printf("\n#% 9d: ",i);
      //print frame index as 4 bytes
      for(unsigned int b=0;b<4;++b){unsigned char o=buffer[b]; printf("%02x ",o);}
      //print frame index as uint32
      {const unsigned int *b=(unsigned int *)buffer; printf("% 9d",*b);}
      printf(" % 11ld",inc);
    }//drop
    //next loop
    prev_index=index;
  }//loop
  printf("\n");
  return 0;
}//main

