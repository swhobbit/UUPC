/*--------------------------------------------------------------------*/
/*    c a t c h e r . c                                               */
/*                                                                    */
/*    Ctrl-Break handler for UUPC/extended                            */
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
 *    $Id: catcher.c 1.3 1993/09/29 04:49:20 ahd Exp $
 *
 *    Revision history:
 *    $Log: catcher.c $
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

#include <stdio.h>
#include <signal.h>
#include <process.h>
#include <stdlib.h>

#if defined(WIN32) || defined(_Windows)
#include "winsock.h"
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "timestmp.h"
#include "catcher.h"
#include "safeio.h"

#if defined(_Windows)
#include "pwinsock.h"
#endif

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

boolean terminate_processing = FALSE;
boolean interactive_processing = TRUE;
boolean norecovery = TRUE;

#if defined(WIN32) || defined(_Windows)
boolean winsockActive = FALSE;      /* Set/reset in ulibip.c          */
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

    if ( ! interactive_processing )
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

#ifdef __OS2__
      signal( sig, (void (__cdecl *)(int))ctrlchandler );
#else
      signal( sig, ctrlchandler );
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
#if 0
#ifdef WIN32
            if (IsNetwork()) {
               if (WSAIsBlocking()) {
                  printmsg(15, "catcher:  sockets are blocking");
                  WSACancelBlockingCall();
               } else {
                   printmsg(15, "catcher:  sockets are not blocking");
               }
            }
#endif
#endif
            break;

        case 'N':
        case 'n':
           safeout("\r\nResuming execution\r\n");
           break;

        default:
           safeout(" -- Invalid response\a");
           ch = INVALID_CHAR;
           break;

      } /* switch  */
   } /* for */

/*--------------------------------------------------------------------*/
/*    The CTRL+C interrupt must be reset to our handler since by      */
/*    default it is reset to the system handler.                      */
/*--------------------------------------------------------------------*/

#ifdef __OS2__
      signal( sig, (void (__cdecl *)(int))ctrlchandler );
#else
      signal( sig, ctrlchandler );
#endif

} /* catcher */
