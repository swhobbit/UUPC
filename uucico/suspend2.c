/*--------------------------------------------------------------------*/
/*    s u s p e n d 2 . c                                             */
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
 *    $Id: suspend2.c 1.5 1993/10/12 01:32:46 ahd Exp $
 *
 *    Revision history:
 *    $Log: suspend2.c $
 * Revision 1.5  1993/10/12  01:32:46  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.4  1993/10/03  20:37:34  ahd
 * Further cleanup for 32 bit environment
 *
 * Revision 1.4  1993/10/03  20:37:34  ahd
 * Further cleanup for 32 bit environment
 *
 * Revision 1.3  1993/09/30  03:06:28  ahd
 * Move suspend signal handler into suspend2
 *
 * Revision 1.2  1993/09/29  04:49:20  ahd
 * Various clean up, with additional messages to user
 * Use unique signal handler
 *
 * Revision 1.1  1993/09/27  00:45:20  ahd
 * Initial revision
 *
 */

/*
 * This modules allows suspending/resuming a running "uucico -r0"
 * from another process if this other process wants to make an outgoing
 * call. This can be an outgoing uucico or any other application.
 * An outgoing uucico can suspend the background one itself while
 * other applications such as terminal emulators will require to
 * be wrapped in a batch files with calls to the uuport utility.
 *
 * The communication between the uuport or outgoing uucico and the
 * background uucico is done via a named pipe. This has the advantage
 * that it also works across a network between two machines if the
 * background uucico runs on a LAN server which makes the modem shareable
 * to other OS/2 machines. Then another machine first suspends the uucico
 * on the server using a named pipe over the network and then requests
 * the modem with a "NET USE" or equivalent operation. After using and
 * disconnecting from the server's modem, it can resume the uucico on
 * the server, again via network pipe.
 */

/*--------------------------------------------------------------------*/
/*       Note that the 32 bit API doesn't handle signals like it's    */
/*       16 bit older cousin.  For now, we support the client of      */
/*       the of the pipe to suspend a 16 bit UUCICO, but a 32 bit     */
/*       UUCICO cannot be suspended.                                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <signal.h>
#include <process.h>
#include <time.h>

#define INCL_DOS
#define INCL_DOSPROCESS
#define INCL_ERRORS
#define INCL_DOSSIGNALS
#include <os2.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hostable.h"
#include "security.h"
#include "dcp.h"
#include "dcpsys.h"
#include "safeio.h"
#include "modem.h"
#include "catcher.h"
#include "pos2err.h"
#include "suspend.h"
#include "ssleep.h"
#include "usrcatch.h"

#define STACKSIZE 8192

boolean suspend_processing = FALSE;

/* This module creates a new thread. Because this thread has a stack
 * segment different from the DGROUP (unlike the main thread) this
 * normally requires compilation with different options. To avoid this
 * (possible because the thread does not use any library routines),
 * we take care not to access any local (stack) variables but only
 * static global variables. There are chances that even accessing stack
 * variables would work (if BP is used) but that can't be ensured.
 * The same is done (to be on the safe side) for the system signal handler.
 */

static HPIPE hPipe;
static char nChar;
static char *portName;

#ifdef __OS2__

static HEV semWait, semFree;
static ULONG postCount;
static ULONG nBytes;

#else

static ULONG semWait, semFree;
static PFNSIGHANDLER old;
static USHORT nAction;
static int nBytes;
typedef USHORT APIRET ;  /* Define older API return type              */

#endif

currentfile();

#ifdef __TURBOC__
#pragma -N-
#else
#pragma check_stack( off )
#endif

#ifndef __OS2__

/*--------------------------------------------------------------------*/
/*       S u s p e n d T h r e a d                                    */
/*                                                                    */
/*       Accept request to release serial port                        */
/*--------------------------------------------------------------------*/

static VOID FAR SuspendThread(VOID)
{


/*--------------------------------------------------------------------*/
/*       Process until we get a request to change the status of       */
/*       the port.                                                    */
/*--------------------------------------------------------------------*/

  for (;;)
  {

#ifdef __OS2__
    if ( DosConnectNPipe(hPipe) )
      break;
#else
    if ( DosConnectNmPipe(hPipe) )
      break;
#endif

    for (;;)
    {
      if ( DosRead(hPipe, &nChar, 1, &nBytes) )
        break;                   /* Quit if an error                  */

      if ( nBytes == 0 )
        break; /* EOF */

/*--------------------------------------------------------------------*/
/*               Handle the received command character                */
/*--------------------------------------------------------------------*/

      switch ( nChar )
      {

        case 'Q': /* query */

          nChar = (char) (suspend_processing ? 'S' : 'R');
          DosWrite(hPipe, &nChar, 1, &nBytes);

          break;

        case 'S': /* suspend */

          if ( suspend_processing ||
               interactive_processing ||
               terminate_processing )
          {
            nChar = 'E';
          }
          else {
            suspend_processing = TRUE;

#ifdef __OS2__
            raise(SIGUSR1);
            nChar = (char) (DosWaitEventSem(&semFree, 20000) ? 'T' : 'O');
#else
            DosFlagProcess(getpid(), FLGP_PID, PFLG_A, 0);
            nChar = (char) (DosSemSetWait(&semFree, 20000) ? 'T' : 'O');
#endif
          } /* else */

          DosWrite(hPipe, &nChar, 1, &nBytes);

          break;

        case 'R': /* release */

          if ( !suspend_processing )
            nChar = 'E';
          else {
            suspend_processing = FALSE;

#ifdef __OS2__
            DosResetEventSem( &semWait, &postCount );
#else
            DosSemClear(&semWait);
#endif
            nChar = 'O';

          } /* else */

          DosWrite(hPipe, &nChar, 1, &nBytes);

          break;

        default:

          nChar = 'U';
          DosWrite(hPipe, &nChar, 1, &nBytes);

          break;

      } /* switch */

    } /* for (;;) */

/*--------------------------------------------------------------------*/
/*         Drop the connection now we're done with this client.       */
/*--------------------------------------------------------------------*/

#ifdef __OS2__
    DosDisConnectNPipe(hPipe);
#else
    DosDisConnectNmPipe(hPipe);
#endif

  } /* for (;;) */

  DosExit(EXIT_THREAD, 0);

} /* SuspendThread */

/*--------------------------------------------------------------------*/
/*       S u s p e n d H a n d l e r                                  */
/*                                                                    */
/*       Signal handler for suspend hander                            */
/*--------------------------------------------------------------------*/

static VOID FAR PASCAL SuspendHandler(USHORT nArg, USHORT nSig)
{

  DosSetSigHandler(SuspendHandler, &old, &nAction,
                   SIGA_ACKNOWLEDGE, SIG_PFLG_A);
  raise(SIGUSR2);

} /* SuspendHandler */

#endif

#ifdef __TURBOC__
#pragma -N
#else
#pragma check_stack( )
#endif

/*--------------------------------------------------------------------*/
/*       s u s p e n d _ i n i t                                      */
/*                                                                    */
/*       Initialize thread to handle port suspension                  */
/*--------------------------------------------------------------------*/

void suspend_init(const char *port )
{

#ifndef __OS2__

  char szPipe[FILENAME_MAX];
  SEL selStack;
  PSZ pStack;
  TID tid;
  APIRET rc;

/*--------------------------------------------------------------------*/
/*      Set up the handler for signals from our suspend monitor       */
/*--------------------------------------------------------------------*/

  rc = DosSetSigHandler(SuspendHandler,
                        &old,
                        &nAction,
                        SIGA_ACCEPT,
                        SIG_PFLG_A);

  if (rc)
  {
    printOS2error( "DosSetSigHandler", rc);
    return;
  }

/*--------------------------------------------------------------------*/
/*                Set up the pipe name to listen upon                 */
/*--------------------------------------------------------------------*/

  strcpy(szPipe, SUSPEND_PIPE);
  portName =  newstr( port );    /* Save for later reference           */
  strcat(szPipe, port );

  printmsg(4,"Creating locking pipe %s", szPipe );

#ifdef __OS2__
  rc = DosCreateNPipe( szPipe,
                     &hPipe,
                     NP_ACCESS_DUPLEX | NP_NOINHERIT | NP_NOWRITEBEHIND,
                     NP_WAIT | NP_READMODE_BYTE | NP_TYPE_BYTE | 1,
                     32,
                     32,
                     5000);

  if (rc)
  {
    printOS2error( "DosCreateNPipe", rc);
    return;
  }

#else
  rc = DosMakeNmPipe(szPipe,
                     &hPipe,
                     NP_ACCESS_DUPLEX | NP_NOINHERIT | NP_NOWRITEBEHIND,
                     NP_WAIT | NP_READMODE_BYTE | NP_TYPE_BYTE | 1,
                     32,
                     32,
                     5000);

  if (rc)
  {
    printOS2error( "DosMakeNmPipe", rc);
    return;
  }
#endif

/*--------------------------------------------------------------------*/
/*       Now allocate memory for the monitor thread which will        */
/*       notify us if some program wants our port.                    */
/*--------------------------------------------------------------------*/

#ifdef __OS2__
  pStack = malloc( STACKSIZE );
#else
  rc = DosAllocSeg(STACKSIZE, &selStack, SEG_NONSHARED);

  if (rc)
  {
    printOS2error( "DosAllocSeg", rc);
    return;
  }

  pStack = (PSZ) MAKEP(selStack, 0) + STACKSIZE -2 ;
#endif

/*--------------------------------------------------------------------*/
/*                    Now fire off the monitor thread                 */
/*--------------------------------------------------------------------*/

  rc = DosCreateThread(SuspendThread, &tid, pStack);

  if (rc)
  {
    printOS2error( "DosCreateThread", rc);
    return;
  }

/*--------------------------------------------------------------------*/
/*                    Finally, our signal handler                     */
/*--------------------------------------------------------------------*/

  if ( signal( SIGUSR2, usrhandler ) == SIG_ERR )
  {
      printmsg( 0, "Couldn't set SIGUSR2\n" );
      panic();
  }

#endif

} /* suspend_init */

/*--------------------------------------------------------------------*/
/*       s u s p e n d _ o t h e r                                    */
/*                                                                    */
/*       Request another UUCICO give up a modem                       */
/*--------------------------------------------------------------------*/

int suspend_other(const boolean suspend,
                  const char *port )
{
  char szPipe[FILENAME_MAX];
  HFILE hPipe;
#ifdef __OS2__
  ULONG nAction, nBytes;
#else
  USHORT nAction, nBytes;
#endif
  UCHAR nChar;
  APIRET rc = 1;
  boolean firstPass = TRUE;
  int result;

  static time_t lastSuspend = 0;
  static char *lastPort = "";    /* Must not be be NULL pointer      */

/*--------------------------------------------------------------------*/
/*                      Open up the pipe to process                   */
/*--------------------------------------------------------------------*/

  strcpy(szPipe, SUSPEND_PIPE);
  strcat(szPipe, port );

/*--------------------------------------------------------------------*/
/*           Try to open the pipe, with one retry if needed           */
/*--------------------------------------------------------------------*/

  while(rc)
  {
      rc = DosOpen((PSZ) szPipe,
                   &hPipe,
                   &nAction,
                   0L,
                   0,
                   FILE_OPEN,
                   OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
                   0);

      if (rc)
      {
        if ( debuglevel >= 4 )          /* No error if no passive UUCICO  */
           printOS2error( "DosOpen", rc); /* So this is only for info  */

        if ((rc == ERROR_PIPE_BUSY) && firstPass )
        {
           firstPass = FALSE;

#ifdef __OS2__
           rc = DosWaitNPipe( szPipe, 5000 ); /* Wait up to 5 sec for pipe  */
           if (rc)
           {
             printOS2error( "DosWaitNPipe", rc);
             return 0;
           } /* if (rc) */
#else
           rc = DosWaitNmPipe( szPipe, 5000 ); /* Wait up to 5 sec for pipe  */
           if (rc)
           {
             printOS2error( "DosWaitNmPipe", rc);
             return 0;
           } /* if (rc) */
#endif

        } /* if */
        else
           return 0;

      } /* if */

   } /* while(rc) */

/*--------------------------------------------------------------------*/
/*       Determine if we need to allow previous suspend of port we    */
/*       issued to finish initializing port.                          */
/*--------------------------------------------------------------------*/

   if ( suspend )
   {
      if ( equal( lastPort, port) )
      {
         time_t diff = time((time_t) NULL) - lastSuspend;
         if ( diff < 5 )
            ssleep( 5 - diff );
      }
   } /* if ( suspend ) */
   else {
      lastPort = newstr(port);
      time( &lastSuspend );
   } /* else */

/*--------------------------------------------------------------------*/
/*       We have an open connect, write the request to the server     */
/*       running as part of the passive UUCICO.                       */
/*--------------------------------------------------------------------*/

   rc = DosWrite(hPipe, suspend ? "S" : "R", 1, &nBytes);
   if (rc)
   {
     printOS2error( "DosWrite", rc);
     DosClose(hPipe);
     return -1;
   }

   if ( nBytes != 1 )
   {
     DosClose(hPipe);
     return -1;
   }

   printmsg(2, "Waiting for background uucico to %s use of %s ...",
               suspend ? "suspend" : "resume",
               port);

/*--------------------------------------------------------------------*/
/*                      Process the server response                   */
/*--------------------------------------------------------------------*/

   rc = DosRead(hPipe, &nChar, 1, &nBytes);

   if (rc)
   {
     printOS2error( "DosRead", rc);
     result = -1;
   }
   else if ( nBytes != 1 )
   {
     printmsg(0,"suspend_other: Error: No data from remote UUCICO");
     result = -2;
   }
   else if ( nChar != 'O' )
   {
     printmsg(0, "Cannot %s background uucico.  Result code was %c",
                 suspend ? "suspend" : "resume",
                 nChar );
     result = -3;
   }
   else
      result = 1;                   /* Success!                       */

/*--------------------------------------------------------------------*/
/*                     Close up and return to caller                  */
/*--------------------------------------------------------------------*/

  DosClose(hPipe);

  return result;

} /* suspend_other */

/*--------------------------------------------------------------------*/
/*       s u s p e n d _ w a i t                                      */
/*                                                                    */
/*       Wait to take the serial port back                            */
/*--------------------------------------------------------------------*/

CONN_STATE suspend_wait(void)
{

#ifdef __OS2__
   return CONN_INITIALIZE;
#else

   APIRET rc;

   printmsg(0,"suspend_wait: Port %s released, program sleeping",
               portName );

#ifdef __OS2__
   rc = DosResetEventSem( &semFree, &postCount);
   if (rc)
      printOS2error( "DosResetEventSem", rc);

   rc = DosWaitEventSem(&semWait, SEM_INDEFINITE_WAIT);
   if (rc)
      printOS2error( "DosWaitEventSem", rc);

#else

   rc = DosSemClear(&semFree);

   if (rc)
      printOS2error( "DosSemClear", rc);

   rc = DosSemSetWait(&semWait, SEM_INDEFINITE_WAIT);
   if (rc)
      printOS2error( "DosSemSetWait", rc);

#endif

   if (rc)
      return CONN_EXIT;
   else
      return CONN_INITIALIZE;

#endif

} /* suspend_wait */
