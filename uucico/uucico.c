/*--------------------------------------------------------------------*/
/*       u u c i c o . c                                              */
/*                                                                    */
/*       UUCICO main program                                          */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: uucico.c 1.11 1995/01/07 16:41:19 ahd Exp $
 *
 *    Revision history:
 *    $Log: uucico.c $
 *    Revision 1.11  1995/01/07 16:41:19  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.10  1994/12/22 00:38:40  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1994/03/09 04:21:30  ahd
 *    Also trap SIGTERM when possible
 *
 * Revision 1.8  1994/02/20  19:11:18  ahd
 * IBM C/Set 2 Conversion, memory leak cleanup
 *
 * Revision 1.7  1994/01/01  19:22:29  ahd
 * Annual Copyright Update
 *
 * Revision 1.6  1993/09/29  04:52:03  ahd
 * Cosmetic cleanup
 *
 * Revision 1.6  1993/09/29  04:52:03  ahd
 * Cosmetic cleanup
 *
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
 *  2 May 1990  Allow set of KWBooleans options via options=        ahd
 * 29 Jul 1990  Change mapping of UNIX to MS-DOS file names        ahd
 * 03 Mar 1992  Replace with new Signal handler for Ctrl C         ahd
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <signal.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "dcp.h"
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
        printerr("signal");
        printmsg( 0, "Couldn't set SIGINT\n" );
        panic();
    }

#if defined(__OS2__) || defined(FAMILYAPI) || defined(WIN32)

    if( signal( SIGTERM, ctrlchandler ) == SIG_ERR )
    {
        printerr("signal");
        printmsg( 0, "Couldn't set SIGTERM\n" );
        panic();
    }

#endif

/*--------------------------------------------------------------------*/
/*                   setup longjmp for error exit's                   */
/*--------------------------------------------------------------------*/

   status = dcpmain(argc, argv);

   exit( status );

} /*main*/
