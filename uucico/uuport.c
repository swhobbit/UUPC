/*--------------------------------------------------------------------*/
/*    u u p o r t . c                                                 */
/*                                                                    */
/*    suspend/resume uupoll/uucico daemon (for OS/2)                  */
/*                                                                    */
/*    Author: Kai Uwe Rommel                                          */
/*                                                                    */
/*--------------------------------------------------------------------*/

/* see uucico\suspend2.c for description */

/* exit codes:
 *   0 = success
 *   1 = incorrect usage of uuport
 *   2 = no uucico running on that modem
 *   3 = communication error between uuport and the uucico
 *   4 = error, uucico cannot currently be suspended
 */

#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#include "lib.h"
#include "timestmp.h"

void usage(void)
{
  printf("Usage: [-sr] [\\\\server\\]modemname\n");
  exit(1);
}

int main(int argc, char **argv)
{
  int file;
  char name[64], pipe[128];
  char *ptr, cmd = 'Q';

  banner( argv );
  
  if ( argc != 2 && argc != 3 )
    usage();

  if ( argv[1][0] != '-' )
    strcpy(name, argv[1]);
  else
  {
    if ( argv[1][2] != 0 )
      usage();
  
    switch ( tolower(argv[1][1]) )
    {
    case 's':
      cmd = 'S';
      break;
    case 'r':
      cmd = 'R';
      break;
    default:
      usage();
    }

    strcpy(name, argv[2]);
  }
  
  if ( strncmp(name, "\\\\", 2) == 0 )  /* server specified ? */
  {
    ptr = name + 2;
    
    if ( (ptr = strchr(ptr, '\\')) == NULL )
    {
      printf("invalid modem '%s' specified.\n", name);
      return 1;
    }

    *ptr = 0;
    strcpy(pipe, name);
    *ptr = '\\';
    strcat(pipe, "\\pipe\\uucico");
    strcat(pipe, ptr);
  }
  else
  {
    strcpy(pipe, "\\pipe\\uucico\\");
    strcat(pipe, name);
  }

  if ( (file = open(pipe, O_RDWR, 0)) == -1 )
  {
    printf("no modem '%s' currently used by any uucico.\n", name);
    return 2;
  }

  if ( cmd != 'Q' )
    printf("waiting for uucico on modem '%s' to %s ... ",
           name, cmd == 'S' ? "suspend" : "resume");
  
  if ( write(file, &cmd, 1) != 1 || read(file, &cmd, 1) != 1 )
  {
    printf("\nerror communicating with uucico.\n");
    return 3;
  }
  
  close(file);

  switch ( cmd )
  {
  case 'O':
    printf("OK\n");
    break;
  case 'S':
    printf("uucico on modem '%s' is suspended.\n", name);
    break;
  case 'R':
    printf("uucico on modem '%s' is active.\n", name);
    break;
  default:
    printf("FAILED\n");
    return 4;
  }

  return 0;
}
