/*--------------------------------------------------------------------*/
/*    c a t c h e r . c                                               */
/*                                                                    */
/*    Ctrl-Break handler for UUPC/extended                            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2000 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: catcher.c 1.27 1999/01/08 02:20:43 ahd Exp $
 *
 *    Revision history:
 *    $Log: catcher.c $
 *    Revision 1.27  1999/01/08 02:20:43  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.26  1999/01/04 03:52:28  ahd
 *    Annual copyright change
 *
 *    Revision 1.25  1998/11/24 03:08:50  ahd
 *    Support on-the-fly debugger entry
 *
 *    Revision 1.24  1998/03/01 01:23:08  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.23  1997/12/13 18:27:55  ahd
 *    Correct RCS keyword
 *
 *    Revision 1.22  1997/12/13 18:26:48  ahd
 *    Use standard RCSID() macro in place of hand declare of variable
 *
 *    Revision 1.21  1997/03/31 06:58:14  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.20  1996/01/01 20:50:00  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.19  1995/03/12 16:42:24  ahd
 *    Correct compiler warnings
 *
 *    Revision 1.18  1995/02/24 00:37:28  ahd
 *    Move panic_rc from catcher.c into bugout.c, keeps catcher from
 *    being included into every module.
 *
 *    Revision 1.17  1995/02/23 15:58:39  ahd
 *    Add RCS id to allow tracing module
 *
 *    Revision 1.16  1995/01/07 16:11:54  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 */

#include "uupcmoah.h"

RCSID("$Id: catcher.c 1.27 1999/01/08 02:20:43 ahd Exp $");

/*--------------------------------------------------------------------*/
/*    Since C I/O functions are not safe inside signal routines,      */
/*    the code uses conditionals to use system-level DOS and OS/2     */
/*    services.  Another option is to set global flags and do any     */
/*    I/O operations outside the signal handler.                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <signal.h>

#include <process.h>

#if defined(WIN32) || defined(_Windows)
#include <windows.h>
#include "winsock.h"
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "timestmp.h"
#include "catcher.h"
#include "safeio.h"

#if defined(_Windows)
#include "pwinsock.h"
#include "winutil.h"
#endif

#if defined(WIN32) && defined(UUCICO)
BOOL AbortComm(void);
KWBoolean IsNetwork(void);
BOOL AbortNetwork(void);
#endif

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

KWBoolean terminate_processing = KWFalse;
KWBoolean interactive_processing = KWTrue;
KWBoolean norecovery = KWTrue;

#if defined(WIN32) || defined(_Windows)
KWBoolean winsockActive = KWFalse;    /* Set/reset in ulibip.c          */

#endif

#define INVALID_CHAR '*'

/*--------------------------------------------------------------------*/
/*    c t r l c h a n d l e r                                         */
/*                                                                    */
/*    Handles SIGINT (CTRL+C) interrupt; from MicroSoft Programmer's  */
/*    Workbench QuickHelp samples                                     */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

void
#ifdef __TURBOC__
__cdecl
#endif
ctrlchandler( int sig )
{
    int ch = INVALID_CHAR;

/*--------------------------------------------------------------------*/
/*                  Disallow CTRL+C during handler.                   */
/*--------------------------------------------------------------------*/

    signal( sig, SIG_IGN );

/*--------------------------------------------------------------------*/
/*          Don't ask if the program doesn't think we should          */
/*--------------------------------------------------------------------*/

    if ( ! interactive_processing || (sig == SIGTERM))
    {

      safeout( "\r\n" );
      safeout( compilen );
      panic_rc = 100;
      terminate_processing = interactive_processing = KWTrue;
      safeout(": *** Termination in progress ***\r\n");

#if defined(WIN32) || defined(_Windows)
      if (winsockActive)
      {
         if (WSAIsBlocking())
         {
            printmsg(15, "catcher:  sockets are blocking");
            WSACancelBlockingCall();
         }
         else {
             printmsg(15, "catcher:  sockets are not blocking");
         }
      } /* if (winsockActive) */
#endif

#if defined(__OS2__) && defined(__BORLANDC__)
      signal( sig, (void (__cdecl *)(int))ctrlchandler );
#else
      signal( sig, ctrlchandler );
#endif

#if defined(WIN32) && defined(UUCICO)
   printmsg(9, "catcher:  calling AbortComm()");
   if (IsNetwork())
      AbortNetwork();
   else
      AbortComm();
#endif

      return;
    }

    if ( terminate_processing )
      safeout( "Termination already in progress ... answer Y to SCRAM program");

/*--------------------------------------------------------------------*/
/*                   Ask user if he/she/it is sure                    */
/*--------------------------------------------------------------------*/

   while ( ch == INVALID_CHAR )
   {
      safeout( "\r\n" );
      safeout( compilen );
#ifdef UDEBUG
      safeout( ": Abort processing (or debug)? (Y/N/D) " );
#else
      safeout( ": Abort processing? (Y/N) " );
#endif
      safeflush();            /* Flush any queued characters          */
      ch = safein();

      switch( ch )
      {

         case 'y':
         case 'Y':
            if ( terminate_processing || norecovery )
            {
               safeout("\n\rProgram aborted.\r\n");
               _exit(100);
            }

            terminate_processing = KWTrue;  /* Controlled shutdown  */
            panic_rc = 100;
            safeout("\n\r*** Termination in progress ***\r\n");
#if defined(WIN32) && defined(UUCICO)
            printmsg(9, "catcher:  calling AbortComm()");
            if (IsNetwork())
               AbortNetwork();
            else
               AbortComm();
#endif
            break;

        case 'D':
        case 'd':
            safeout("\r\nFiring breakpoint\r\n");
            BREAKPOINT;
            break;

        case 'N':
        case 'n':
           safeout("\r\nResuming execution\r\n");
           break;

        default:
           safeout(" -- Invalid response");
           if ( ! bflag[F_SUPPRESSBEEP] )
              safeout("\a");
           ch = INVALID_CHAR;
           break;

      } /* switch  */
   } /* for */

/*--------------------------------------------------------------------*/
/*    The CTRL+C interrupt must be reset to our handler since by      */
/*    default it is reset to the system handler.                      */
/*--------------------------------------------------------------------*/

#if defined(__OS2__) && defined(__BORLANDC__)
      signal( sig, (void (__cdecl *)(int))ctrlchandler );
#else
      signal( sig, ctrlchandler );
#endif

} /* catcher */
