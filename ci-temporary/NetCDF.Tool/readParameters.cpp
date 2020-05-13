#include <iostream>
#include <string>

//ArgP
#include <error.h>
#include <argp.h>

//NetCDF
#include <netcdfcpp.h>

//DEBUG LEVEL
#define cimg_debug 12

#include "struct_parameter_NetCDF.h"

#define VERSION "v0.4.5d"

using namespace std;

//Program option/documentation
//{argp
//! [argp] version of program
const char *argp_program_version=VERSION"\
\n  CParameterNetCDF."CDL_PARAMETER_VERSION"\
\n  NcTypeInfo."NETCDF_TYPE_INFO_VERSION;
//! [argp] author of program
const char *argp_program_bug_address="sebastien.coudert@ganil.fr";
//! [argp] documentation of program
static char doc[]=
   "read parameters from .NC file (should be compiled from .CDL using ncgen)\
\n  readParameters."VERSION"\
\n  CParameterNetCDF."CDL_PARAMETER_VERSION"\
\n  NcTypeInfo."NETCDF_TYPE_INFO_VERSION"\
\n\
examples:\n\
  readParameters --help\n\
  readParameters -v\n\
  ncgen parameters.cdl -o parameters.nc && readParameters -v -p parameters.nc\n\
  readParameters -V\n\
  readParameters --usage";

//! [argp] A description of the arguments
static char args_doc[] = "";

//! [argp] The options and its description
static struct argp_option options[]=
{
  {"verbose",   'v', 0, 0,               "Produce verbose output" },
  {"parameters",'p', "parameters.nc", 0, "input file containing parameters, e.g. k,m,alpha." },
//default options
  { 0 }
};//options (CLI)

//! [argp] Used by main to communicate with parse_option
struct arguments
{
  //! verbose mode (boolean)
  int verbose;
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
    case 'p':
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
  printf (".verbose=%s\n.integer=%d\n.string=%s\n"
  , p_arguments->verbose?"yes":"no"
  , p_arguments->string
  );
}//print_args

//! [argp] setup argp parser
static struct argp argp = { options, parse_option, args_doc, doc };

//}argp

int main(int argc, char **argv)
{
  //CLI arguments
  struct arguments arguments;
  arguments.verbose=0;
  arguments.string="parameters.nc";

//! - print default option values (static)
  if(0)//0 or 1
  {
    printf("default values:\n");
    print_args(&arguments);
  }//print default option values

//! - Parse arguments (see parse_option) and eventually show usage, help or version and exit for these lasts
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  ///print option values
  if(arguments.verbose)
  {
    printf("command line option values:\n");
    print_args(&arguments);
  }//print default option values

  ///file name
  string fi=arguments.string;
  ///parameters
  int k,m;
  double alpha;

  ///parameter class
  CParameterNetCDF fp;
  //open file
  int error=fp.loadFile((char *)fi.c_str());
  if(error){std::cerr<<"loadFile return "<< error <<std::endl;return error;}

///trapezoid
  int process; string process_name="trapezoid";
  //load process variable
  error=fp.loadVar(process,&process_name);
  if(error){cerr<<"loadVar return "<< error <<endl;return error;}
std::cout<<process_name<<"="<<process<<std::endl;
  ///k
  string attribute_name="k";
  if (error = fp.loadAttribute(attribute_name,k)!=0){
    std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
    return error;
  }
std::cout<<attribute_name<<"="<<k<<std::endl;

  ///m
  attribute_name="m";
  if (error = fp.loadAttribute(attribute_name,m)!=0){
    std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
    return error;
  }
std::cout<<attribute_name<<"="<<m<<std::endl;

  ///alpha
  attribute_name="alpha";
  if (error = fp.loadAttribute(attribute_name,alpha)!=0){
    std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
    return error;
  }
std::cout<<attribute_name<<"="<<alpha<<std::endl;

///energy
  process_name="energy";
  error=fp.loadVar(process,&process_name);
  if(error){cerr<<"loadVar return "<< error <<endl;return error;}
std::cout<<process_name<<"="<<process<<std::endl;
  ///alpha
  attribute_name="n";
  vector<string> n;
  if (error = fp.loadAttribute(attribute_name,n)!=0){
    std::cerr<< "Error while loading "<<process_name<<":"<<attribute_name<<" attribute"<<std::endl;
    return error;
  }
std::cout<<attribute_name<<"=";
for(int i=0;i<n.size();++i) std::cout<<n[i]<<", ";
std::cout<<";"<<std::endl;

  return 0;
}

