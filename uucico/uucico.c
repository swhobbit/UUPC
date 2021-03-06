/*--------------------------------------------------------------------*/
/*       u u c i c o . c                                              */
/*                                                                    */
/*       UUCICO main program                                          */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: uucico.c 1.22 2001/03/12 13:55:29 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: uucico.c $
 *    Revision 1.22  2001/03/12 13:55:29  ahd
 *    Annual copyright update
 *
 *    Revision 1.21  2000/05/12 12:32:55  ahd
 *    Annual copyright update
 *
 *    Revision 1.20  1999/01/08 02:21:01  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.19  1999/01/04 03:53:57  ahd
 *    Annual copyright change
 *
 *    Revision 1.18  1998/05/11 01:20:48  ahd
 *    Correct compile warning
 *
 *    Revision 1.17  1998/04/19 23:55:58  ahd
 *    *** empty log message ***
 *
 *    Revision 1.16  1998/03/01 01:41:01  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.15  1997/04/24 01:36:23  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1996/01/27 16:49:22  ahd
 *    Add SIGBREAK handler
 *
 *    Revision 1.13  1996/01/01 21:20:09  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1995/02/14 04:38:42  ahd
 *    Correct problems with directory processing under NT
 *
 *    Revision 1.11  1995/01/07 16:41:19  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
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

RCSID("$Id: uucico.c 1.22 2001/03/12 13:55:29 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program for UUCICO                                         */
/*--------------------------------------------------------------------*/

main( int argc, char *argv[])
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

#if defined(__OS2__)

    if( signal( SIGBREAK , ctrlchandler ) == SIG_ERR )
    {
        printerr("signal");
        printmsg( 0, "Couldn't set SIGBREAK\n" );
        panic();
    }

#endif

/*--------------------------------------------------------------------*/
/*                   setup longjmp for error exit's                   */
/*--------------------------------------------------------------------*/

   status = dcpmain(argc, argv);

   exit( status );
   return status;

} /*main*/
