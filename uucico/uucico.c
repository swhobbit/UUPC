/*--------------------------------------------------------------------*/
/*       u u c i c o . c                                              */
/*                                                                    */
/*       UUCICO main program                                          */
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
 *    $Id: uucico.c 1.5 1993/09/20 04:46:34 ahd Exp $
 *
 *    Revision history:
 *    $Log: uucico.c $
 * Revision 1.5  1993/09/20  04:46:34  ahd
 * OS/2 2.x support (BC++ 1.0 support)
 * TCP/IP support from Dave Watt
 * 't' protocol support
 *
 * Revision 1.4  1993/07/31  16:27:49  ahd
 * Changes in support of Robert Denny's Windows support
 *
 *
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

currentfile();

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program for UUCICO                                         */
/*--------------------------------------------------------------------*/

void main( int argc, char *argv[])
{
   int status;

   logfile = stderr;

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

   PushDir(E_spooldir);
   atexit( PopDir );

/*--------------------------------------------------------------------*/
/*                   setup longjmp for error exit's                   */
/*--------------------------------------------------------------------*/

   status = dcpmain(argc, argv);

   exit( status );

} /*main*/
