#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int main()
{
  int clientSocket, portNum, nBytes;
  char buffer[1024];
  struct sockaddr_in serverAddr;
  socklen_t addr_size;

  //Create UDP socket
  clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

  //Configure settings in address struct
  serverAddr.sin_family = AF_INET;
serverAddr.sin_port = htons(20485);
serverAddr.sin_addr.s_addr = inet_addr("10.10.15.1");
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  //Initialize size variable to be used later on
  addr_size = sizeof serverAddr;

    for(int i=0;i<1024;++i) buffer[i]=(char)i+1;
    buffer[1023]='\0';
    nBytes = strlen(buffer) + 1;
  printf("size=%d.\n",nBytes);

//  while(1)
  for(int i=0;i<123456;++i)
  {
printf("i=%d\r",i);
      buffer[0]=0x12;
      buffer[1]=0x34;
      buffer[2]=0x56;
      buffer[3]=0x78+(unsigned char)((i<123)?i:i+12);
    //Send message to server
    sendto(clientSocket,buffer,nBytes,0,(struct sockaddr *)&serverAddr,addr_size);
usleep(1234);
  }
printf("\n");
  return 0;
}

