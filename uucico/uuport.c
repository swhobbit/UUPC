/*--------------------------------------------------------------------*/
/*    u u p o r t . c                                                 */
/*                                                                    */
/*    suspend/resume uupoll/uucico daemon (for OS/2)                  */
/*                                                                    */
/*    Author: Kai Uwe Rommel                                          */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Copyright (c) 1993 by Kai Uwe Rommel                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1993 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: uuport.c 1.3 1993/10/01 02:36:45 dmwatt Exp $
 *
 *    Revision history:
 *    $Log: uuport.c $
 * Revision 1.3  1993/10/01  02:36:45  dmwatt
 * Correct increment of pointer to pipe name
 *
 * Revision 1.2  1993/09/29  04:49:20  ahd
 * Use standard port name
 *
 */

/* see uucico\suspend2.c for description */

/* exit codes:
 *   0 = success
 *   1 = incorrect usage of uuport
 *   2 = no uucico running on that port
 *   3 = communication error between uuport and the uucico
 *   4 = error, uucico cannot currently be suspended
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "timestmp.h"
#define  NO_SUSPEND_FUNCTIONS
#include "suspend.h"

/*--------------------------------------------------------------------*/
/*       u s a g e                                                    */
/*                                                                    */
/*       Report program usage                                         */
/*--------------------------------------------------------------------*/

static void usage(const char *program)
{
  printf("Usage:\t%s\t[-sr] [\\\\server\\]portname\n", program);
  exit(1);

} /* usage */

/*--------------------------------------------------------------------*/
/*       m a i n                                                      */
/*                                                                    */
/*       Main program, of course                                      */
/*--------------------------------------------------------------------*/

int main(int argc, char **argv)
{
  int file;
  char name[64], pipe[FILENAME_MAX];
  char *ptr, cmd = 'Q';

  banner( argv );

  if ( argc < 2 )
    usage(argv[0]);

  if ( argv[1][0] != '-' )
    strcpy(name, argv[1]);
  else
  {
    if (( argv[1][2] != 0 ) || ( argc < 3 ))
      usage(argv[0]);

    switch ( tolower(argv[1][1]) )
    {
       case 's':
         cmd = 'S';
         break;
       case 'r':
         cmd = 'R';
         break;
       default:
         usage(argv[0]);
    }

    strcpy( name, argv[2] );

  }

  if ( strncmp(name, "\\\\", 2) == 0 )  /* server specified ? */
  {
    ptr = name + 2;

    if ( (ptr = strchr(ptr, '\\')) == NULL )
    {
      printf("invalid port '%s' specified.\n", name);
      return 1;
    }

    *ptr++ = '\0';
    strcpy(pipe, name);
    strcat(pipe, SUSPEND_PIPE );
    strcat(pipe, ptr);
  }
  else
  {
    strcpy(pipe, SUSPEND_PIPE );
    strcat(pipe, name);
  }

  if ( (file = open(pipe, O_RDWR, 0)) == -1 )
  {
    printf("No port '%s' currently used by any uucico.\n", name);
    return 2;
  }

  if ( cmd != 'Q' )
    printf("Waiting for uucico on port '%s' to %s ... ",
           name, cmd == 'S' ? "suspend" : "resume");

  if ( write(file, &cmd, 1) != 1 )
  {
    printf("\nError sending message to uucico.\n");
    return 3;
  }
  if ( read(file, &cmd, 1) != 1 )
  {
    printf("\nError reading message from uucico.\n");
    return 3;
  }

  close(file);

  switch ( cmd )
  {
     case 'O':
       printf("OK\n");
       break;
     case 'S':
       printf("uucico on port '%s' is suspended.\n", name);
       break;
     case 'R':
       printf("uucico on port '%s' is active.\n", name);
       break;
     default:
       printf("FAILED\n");
       return 4;
  }

  return 0;

} /* main */
