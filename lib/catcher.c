/*--------------------------------------------------------------------*/
/*    c a t c h e r . c                                               */
/*                                                                    */
/*    Ctrl-Break handler for UUPC/extended                            */
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
 *    $Id: catcher.c 1.13 1994/05/06 03:55:50 ahd Exp $
 *
 *    Revision history:
 *    $Log: catcher.c $
 *     Revision 1.13  1994/05/06  03:55:50  ahd
 *     Force SIGTERM to handled non-interactively, because it normally
 *     comes from a OS/2 kill process command (such as close window).
 *
 *     Revision 1.12  1994/02/19  04:47:57  ahd
 *     Use standard first header
 *
 *     Revision 1.11  1994/02/19  04:01:06  ahd
 *     Use standard first header
 *
 *     Revision 1.10  1994/02/18  23:16:44  ahd
 *     Use standard first header
 *
 *     Revision 1.9  1994/02/14  01:03:56  ahd
 *     Use system include syntax for windows.h
 *
 *     Revision 1.8  1994/01/01  19:00:36  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.7  1993/12/29  03:34:37  dmwatt
 *     Add special multi-threaded abort code for Windows NT UUCICO
 *
 *     Revision 1.6  1993/12/26  16:20:17  ahd
 *     Dummy code for MessageBox under Windows
 *
 *     Revision 1.5  1993/12/23  03:11:17  rommel
 *     OS/2 32 bit support for additional compilers
 *
 *     Revision 1.4  1993/10/12  00:49:39  ahd
 *     Normalize comments
 *
 *     Revision 1.3  1993/09/29  04:49:20  ahd
 *     Use actual signal handler number for resetting handler
 *
 *     Revision 1.2  1993/09/20  04:38:11  ahd
 *     TCP/IP support from Dave Watt
 *     't' protocol support
 *     OS/2 2.x support
 *
 */

/*--------------------------------------------------------------------*/
/*    Since C I/O functions are not safe inside signal routines,      */
/*    the code uses conditionals to use system-level DOS and OS/2     */
/*    services.  Another option is to set global flags and do any     */
/*    I/O operations outside the signal handler.                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

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
boolean IsNetwork(void);
BOOL AbortNetwork(void);
#endif

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

boolean terminate_processing = FALSE;
boolean interactive_processing = TRUE;
boolean norecovery = TRUE;

#if defined(WIN32) || defined(_Windows)
boolean winsockActive = FALSE;      /* Set/reset in ulibip.c          */

currentfile();
#endif

int panic_rc = 69;

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
      terminate_processing = interactive_processing = TRUE;
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
      safeout( ": Abort processing? (Y/N) " );
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

            terminate_processing = TRUE;  /* Controlled shutdown  */
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
