#include "arg_parse.h"
#include <stdio.h>

// Holds the doc for the progam
const char doc[] = "Doc for the program";

// Holds the doc for the arguments
const char doc_args[] = "PATH";


/* Holds all possible options for the program
  {long_name, short_option_or_key, option_arg, arg_flag, description}*/
static struct argp_option options[] =
{
  {"output", 'o', "FILE", 0, "Output to File"},
  {"brightness", 'b', "VALUE", 0, "Adjust the brightness"},
  { 0 } // Ends always with zero!
};


// Parse function
error_t parse_option(int key, char *arg, struct argp_state *state)
{
  arguments *arguments = state->input;

  switch (key)
  {
    // Option for output
    case 'o':
      arguments->output = arg;
      break;


    // Option for brightness
    case 'b':
      arguments->brightness_adjust = arg;
      break;

    // Key for the arguments | input without an option in front
    case ARGP_KEY_ARG:
      // First argument
      if(state->arg_num == 0)
      {
        arguments->input_path = arg;
      }
      break;


    case ARGP_KEY_END:
      //checks for Path and the triggerd option
      if(state->arg_num == 1)
        //TODO
      break;


    // Key if no option or argument hand over
    case ARGP_KEY_NO_ARGS:
      argp_usage(state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
  }

  return 0;
}


struct argp argp = {options, parse_option, doc_args, doc};
