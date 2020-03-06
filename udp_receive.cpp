//CoolImage
#include "CImg.h"
//C++ base
#include <iostream>
#include <string>
#include <vector>

//C base
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

//ArgP
#include <error.h>
#include <argp.h>

// UDP point to point test

//! \todo drop of exactly 2^32 should not be taken into drops
//! \todo . add CImg   for option and buffer
//! \todo add NetCDF for storing both frame index and increment
//! \todo tests: ml507, RockAMali, numexo2

#define VERSION "v0.1.1f"

using namespace cimg_library;

//Program option/documentation
//{argp
//! [argp] version of program
const char *argp_program_version=VERSION;
//! [argp] author of program
const char *argp_program_bug_address="sebastien.coudert@ganil.fr";
//! [argp] documentation of program
static char doc[]=
   "ArgParse: ArgP minimal/template program\
\n  ArgParse."VERSION"\
\n\
examples:\n\
  ArgParse --help\n\
  ArgParse -v -i 1234\n\
  ArgParse -v -i 12 -s -e\n\
  ArgParse -V\n\
  ArgParse --usage";

//! [argp] A description of the arguments
static char args_doc[] = "";

//! [argp] The options and its description
static struct argp_option options[]=
{
  {"verbose",   'v', 0, 0,        "Produce verbose output" },
  {"endian",    'e', 0, 0,        "do not swap endianess, by default it is done if needed (arch. dep.)" },
  {"simulation",'s', 0, 0,        "frame simulation" },
  {"debug",     'D', 0, 0,        "debug output" },
  {"integer",   'i', "VALUE",  0, "number of iteration" },
  {"string",    'S', "STRING", 0, "get string" },
//default options
  { 0 }
};//options (CLI)

//! [argp] Used by main to communicate with parse_option
struct arguments
{
  //! verbose mode (boolean)
  int verbose;
  //! swap endianess mode (boolean)
  int no_endian;
  //! simulation mode (boolean)
  int simulation;
  //! debug mode (boolean)
  int debug;
  //! integer value
  int integer;
  //! string value
  char* string;
};//arguments (CLI)

//! [argp] Parse a single option
static error_t
parse_option(int key, char *arg, struct argp_state *state)
{
  //Get the input argument from argp_parse
  struct arguments *arguments=(struct arguments *)(state->input);
  switch (key)
  {
    case 'v':
      arguments->verbose=1;
      break;
    case 'e':
      arguments->no_endian=1;
      break;
    case 's':
      arguments->simulation=1;
      break;
    case 'D':
      arguments->debug=1;
      break;
    case 'i':
      arguments->integer=atoi(arg);
      break;
    case 'S':
      arguments->string=arg;
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }//switch
  return 0;
}//parse_option


//! [argp] print argument values
void print_args(struct arguments *p_arguments)
{
  printf (".verbose=%s\n.swap_endianess=%s\n.simulation=%s\n.debug=%s\n.count=%d\n.string=%s\n"
  , p_arguments->verbose?"yes":"no"
  , p_arguments->no_endian?"no":"yes"
  , p_arguments->simulation?"yes":"no"
  , p_arguments->debug?"yes":"no"
  , p_arguments->integer
  , p_arguments->string
  );
}//print_args

//! [argp] setup argp parser
static struct argp argp = { options, parse_option, args_doc, doc };

//}argp

//! CLI option parse and ...
int main(int argc, char **argv)
{
  //CLI arguments
  struct arguments arguments;
  arguments.verbose=0;
  arguments.no_endian=0;
  arguments.simulation=0;
  arguments.debug=0;
  arguments.integer=123;
  arguments.string="ABC";

//! - print default option values (static)
  if(0)//0 or 1
  {
    printf("default values:\n");
    print_args(&arguments);
  }//print default option values

  ///command arguments, i.e. CLI option
  cimg_usage(std::string("receive UDP frame.\n" \
  " It uses different GNU libraries (see --info option)\n\n" \
  " usage: ./receive -h\n" \
  "        ./receive -s 1024 -n 123 -o result.nc\n" \
  "\n version: "+std::string(VERSION) + 
#ifdef DO_NETCDF
  "\n          CImg_NetCDF."+std::string(CIMG_NETCDF_VERSION) + 
  "\n          CParameterNetCDF."+std::string(CDL_PARAMETER_VERSION)+
  "\n          NcTypeInfo."+std::string(NETCDF_TYPE_INFO_VERSION)+
#endif //NetCDF
  "\n compilation date:" \
  ).c_str());//cimg_usage

  const int width=cimg_option("-s",1024, "size   of udp buffer");

//! - Parse arguments (see parse_option) and eventually show usage, help or version and exit for these lasts
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  ///print option values
  if(arguments.verbose)
  {
    printf("command line option values:\n");
    print_args(&arguments);
  }//print default option values

  //! behaviour booleans
  const char endian_swap=!arguments.no_endian;
  const char udp=!arguments.simulation;
  const char debug=arguments.debug;
  //! number of iteration
  const unsigned long max_iter=arguments.integer;

  //UDP related
  int udpSocket, nBytes=4;
  char buffer[2048];
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;

  //create UDP socket
  udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

  //configure settings in address struct
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(20485);
  serverAddr.sin_addr.s_addr = inet_addr("10.10.17.202");
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  //bind socket with address struct
  bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
  //initialize size variable to be used later on
  addr_size = sizeof serverStorage;

  //index
  unsigned int index=0;
  unsigned int prev_index=0;
  //index increment
  long inc=0;
  unsigned long count_drop=0;
  unsigned long count_drops=0;
  //loop index
  unsigned long i=0;
  if(arguments.verbose) if(!endian_swap) printf("information: NO swap endianess\n");
  if(arguments.verbose) printf("information: simulate UDP frame\n");
  if(arguments.verbose) printf("information: increment on first frame is supposed to be ok (not counted as a drop)\n");
  printf("#% 12s: % 11s % 10s % 11s; drop: % 12s      , % 12s","index iter.","frame hex","dec.","increment","drop","index drops");
//  while(1)
  for(;i<max_iter;++i)
  {
    if(udp)
    {//receiving UDP frame
      if(debug) printf("\ndebug: wait for UDP frame");
      //! receive any incoming UDP datagram. Address and port of requesting client will be stored on serverStorage variable
      nBytes = recvfrom(udpSocket,buffer,2048,0,(struct sockaddr *)&serverStorage, &addr_size);
    }//UDP
    else
    {//draft simulation
      //! increment frame index (on first byte only), and simulate a frame drop at loop index 123 (note: looping over size of byte yield to -255 step)
      buffer[0]=0x12;
      buffer[1]=0x34;
      buffer[2]=0x56;
      buffer[3]=0x78+(unsigned char)((i<123)?i:i+12);
    }//simulation
    //get frame index as first uint32 of buffer content
    {const unsigned int *b=(unsigned int *)buffer;index=(!endian_swap)?(*b):ntohl(*b);}//frame index (with endianess)
    //check increment
    inc=(long)index-(long)prev_index;
    if( (inc!=1) || debug)
    {
      //print loop index
      printf("\n#% 12ld: ",i);
      //print frame index as 4 bytes (as is in net buffer)
      for(unsigned int b=0;b<4;++b){unsigned char o=buffer[b]; printf("%02x ",o);}
      //print frame index as uint32 (with endianess swap)
      printf("% 10u",index);
    }//drop|debug
    if(inc!=1)
    {//frame drop
      //drops
      if(i>0)
      {
        ++count_drop;
        count_drops+=abs(inc-1);//normal increment is 1
      }
      //print drop related
      printf(" % 11ld",inc);
      if(count_drop>0) printf("; drop: % 12lu drops, % 12lu index drops",count_drop,count_drops);
    }//drop
    //next loop
    prev_index=index;
  }//loop
  printf("\n");
  //summary of drops
  if(count_drops==0) printf("test pass: zero drop");
  else printf("test fail: in total, % 12ld drops, % 12ld index drops",count_drop,count_drops);
  printf(" on %d BoF (Bytes of Frame).\n",nBytes);
  return 0;
}//main

