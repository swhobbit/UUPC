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
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: uuport.c 1.8 1993/12/26 16:20:17 ahd Exp $
 *
 *    Revision history:
 *    $Log: uuport.c $
 * Revision 1.8  1993/12/26  16:20:17  ahd
 * Windows NT support
 * Use enumerated type for command verbs and responses
 * Add -e flag to command line
 *
 * Revision 1.7  1993/12/23  03:17:55  rommel
 * OS/2 32 bit support for additional compilers
 *
 * Revision 1.6  1993/12/02  02:25:12  ahd
 * Correct auto-close of window under Windows 3.1
 *
 * Revision 1.5  1993/11/30  04:18:14  ahd
 * Automatically close program window after execution under Windows
 *
 * Revision 1.4  1993/10/07  22:56:45  ahd
 * Adding missing copy of port name
 *
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

#ifdef _Windows
#include <windows.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "timestmp.h"
#define  NO_SUSPEND_FUNCTIONS
#include "suspend.h"

#ifdef _Windows
#include "winutil.h"
#include "logger.h"
#endif

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
  char *ptr, cmd = SUSPEND_QUERY;

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
         cmd = SUSPEND_SLEEP;
         break;

       case 'r':
         cmd = SUSPEND_RESUME;
         break;

       case 'e':
         cmd = SUSPEND_EXIT;     /* UUCICO should exit            */

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

    *ptr = '\0';
    strcpy(pipe, name);
    strcat(pipe, SUSPEND_PIPE );
    *ptr++ = '\\';
    strcat(pipe, ptr);
  }
  else
  {
    strcpy(pipe, SUSPEND_LOCAL );   /* For compatbability with NT */
    strcat(pipe, SUSPEND_PIPE );
    strcat(pipe, name);
  }

#if defined(_Windows)

/*--------------------------------------------------------------------*/
/*       Under Windows 3.1, if we get this far, automatically         */
/*       close the Window when done                                   */
/*--------------------------------------------------------------------*/

   openlog( NULL );
   atexit( CloseEasyWin );               /* Auto-close EasyWin on exit  */

#endif

  if ( (file = open(pipe, O_RDWR, 0)) == -1 )
  {
    printf("No port '%s' currently used by any uucico.\n", name);
    return 2;
  }

  if ( cmd != SUSPEND_QUERY )
  {
    printf("Waiting for uucico on port '%s' to %s ... ",
           name, cmd == SUSPEND_SLEEP ? "suspend" : "resume");
    fflush(stdout);
  }

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
     case SUSPEND_OKAY:
       printf("OK\n");
       break;

     case SUSPEND_WAITING:
       printf("uucico on port '%s' is suspended.\n", name);
       break;

     case SUSPEND_ACTIVE:
       printf("uucico on port '%s' is active.\n", name);
       break;

     case SUSPEND_BUSY:
       printf("\nuucico on port '%s' is busy and rejected request.\n", name);
       break;

     case SUSPEND_ERROR:
       printf("\nuucico on port '%s' had a system error processing request.\n",
               name);
       break;

     default:
       printf("\nUUCICO returned error code '%c'\n", cmd);
       return 4;
  }

  return 0;

} /* main */
