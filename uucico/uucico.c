/*
 * History:4,1
 * Mon May 15 19:56:44 1989 Add c_break handler                   ahd
 * 20 Sep 1989 Add check for SYSDEBUG in MS-DOS environment       ahd
 * 22 Sep 1989 Delete kermit and password environment
 *             variables (now in password file).                  ahd
 * 30 Apr 1990  Add autoedit support for sending mail              ahd
 *  2 May 1990  Allow set of booleans options via options=         ahd
 * 29 Jul 1990  Change mapping of UNIX to MS-DOS file names        ahd
 * 03 Mar 1992  Replace with new Signal handler for Ctrl C         ahd
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <dos.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "dcp.h"
#include "hlib.h"
#include "hostable.h"
#include "security.h"
#include "pushpop.h"
#include "timestmp.h"
#include "catcher.h"

#ifdef WIN32
#include "win32.h"
#endif

currentfile();

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program for UUCICO                                         */
/*--------------------------------------------------------------------*/

void main( int argc, char *argv[])
{
   int status;

/*--------------------------------------------------------------------*/
/*          Report our version number and date/time compiled          */
/*--------------------------------------------------------------------*/

   banner( argv );

#if defined(__CORE__)
   copywrong = strdup(copyright);
   checkref(copywrong);
#endif

   if (!configure( B_UUCICO ))
      panic();

/*--------------------------------------------------------------------*/
/*                        Trap control C exits                        */
/*--------------------------------------------------------------------*/

    if( signal( SIGINT, ctrlchandler ) == SIG_ERR )
    {
        printmsg( 0, "Couldn't set SIGINT\n" );
        panic();
    }

/*--------------------------------------------------------------------*/
/*          In Windows/NT, set the console input mode to non-linebased*/
/*--------------------------------------------------------------------*/
#ifdef WIN32
   setstdinmode();
#endif


/*--------------------------------------------------------------------*/
/*                         Set our time zone                          */
/*--------------------------------------------------------------------*/

   if (getenv("TZ") == NULL )
   {
      printmsg(0,"Environment variable TZ must be set!");
      panic();
   }
   tzset();                      /* Set up time zone information  */

   PushDir(E_spooldir);
   atexit( PopDir );

/*--------------------------------------------------------------------*/
/*                   setup longjmp for error exit's                   */
/*--------------------------------------------------------------------*/

   status = dcpmain(argc, argv);

   exit( status );
} /*main*/
