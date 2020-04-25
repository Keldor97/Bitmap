#ifndef ARG_PARSE_H
#define ARG_PARSE_H

#include <argp.h>


// This struct hold the input value for triggerd options
typedef struct _arguments
{
  char* input_path;
  char* brightness_adjust;
  char* output;
} arguments;

struct argp argp;

#endif
