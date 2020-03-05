#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

int main(){
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

//  while(1)
  for(unsigned int i=0;i<1234;++i)
  {
    /* Try to receive any incoming UDP datagram. Address and port of 
      requesting client will be stored on serverStorage variable */
    nBytes = recvfrom(udpSocket,buffer,2048,0,(struct sockaddr *)&serverStorage, &addr_size);
    printf("\n#% 9d: ",i);
    for(unsigned int b=0;b<4;++b){unsigned char o=buffer[b]; printf("%02x ",o);}
  }//loop
  printf("\n");
  return 0;
}//main

