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
 *    $Id: suspend2.c 1.26 2001/03/12 13:54:49 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: suspend2.c $
 *    Revision 1.26  2001/03/12 13:54:49  ahd
 *    Annual copyright update
 *
 *    Revision 1.25  2000/05/12 12:32:00  ahd
 *    Annual copyright update
 *
 *    Revision 1.24  1999/01/08 02:20:56  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.23  1999/01/04 03:53:30  ahd
 *    Annual copyright change
 *
 *    Revision 1.22  1998/03/01 01:40:21  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.21  1997/04/24 01:35:08  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.20  1996/01/01 21:22:30  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.19  1995/07/21 13:27:00  ahd
 *    If modem is unable to dial, be sure to resume suspended UUCICO if needed
 *
 *    Revision 1.18  1995/01/07 16:39:55  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.17  1994/12/22 00:36:14  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.16  1994/12/09 03:50:40  rommel
 *    Correct errors with back-to-back uses of port failing
 *
 *        Revision 1.15  1994/10/24  23:40:36  rommel
 *        Prevent suspend processing hanging from rapid closing and opening
 *        of the same file.
 *
 *        Revision 1.14  1994/10/23  23:29:44  ahd
 *        Better control of suspension of processing
 *
 *        Revision 1.13  1994/05/01  21:59:06  dmwatt
 *        Trap errors from failure of suspend_init to create pipe
 *
 * Revision 1.12  1994/02/19  05:10:23  ahd
 * Use standard first header
 *
 * Revision 1.11  1994/01/01  19:21:01  ahd
 * Annual Copyright Update
 *
 * Revision 1.10  1993/12/26  16:20:17  ahd
 * Use enumerated type for command verbs and responses
 *
 * Revision 1.9  1993/12/24  05:12:54  ahd
 * 32 bit support for suspending UUCICO
 *
 * Revision 1.8  1993/12/23  01:41:15  rommel
 * 32 bit support for suspending port access
 *
 * Revision 1.7  1993/11/07  19:09:56  ahd
 * Zap name in error message to show pipe name
 *
 * Revision 1.6  1993/10/24  21:51:14  ahd
 * Delay if suspending/resuming same port multiple times
 *
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
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <limits.h>
#include <signal.h>
#include <process.h>

#define INCL_DOS
#define INCL_DOSPROCESS
#define INCL_ERRORS
#define INCL_DOSSIGNALS
#include <os2.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

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

KWBoolean suspend_processing = KWFalse;

/* Note for the 16-bit version only:
 *
 * This module creates a new thread. Because this thread has a stack
 * segment different from the DGROUP (unlike the main thread) this
 * normally requires compilation with different options. To avoid this
 * (possible because the thread does not use any library routines),
 * we take care not to access any local (stack) variables but only
 * static global variables. There are chances that even accessing stack
 * variables would work (if BP is used) but that can't be ensured.
 * The same is done (to be on the safe side) for the system signal handler.
 *
 * And since most of the code is used for the 32-bit version too,
 * we just keep it that way.
 */

static HPIPE hPipe;
static char nChar;
static char *portName;

#ifdef BIT32ENV

static HEV semWait, semFree, semReady;
static ULONG postCount;
static BOOL bDummyKill;

typedef ULONG U_INT;

#else

static ULONG semWait, semFree, semReady;
static PFNSIGHANDLER old;
static USHORT nAction;

typedef USHORT APIRET;   /* Define older API return type              */
typedef USHORT U_INT;

#endif

static U_INT nBytes;

RCSID("$Id: suspend2.c 1.26 2001/03/12 13:54:49 ahd v1-13k $");

#ifdef __TURBOC__
#pragma -N-
#else
#pragma check_stack( off )
#endif

/*--------------------------------------------------------------------*/
/*       S u s p e n d T h r e a d                                    */
/*                                                                    */
/*       Accept request to release serial port                        */
/*--------------------------------------------------------------------*/

#ifdef BIT32ENV
static VOID APIENTRY SuspendThread(ULONG nArg)
#else
static VOID FAR SuspendThread(VOID)
#endif
{
  static APIRET rc;

/*--------------------------------------------------------------------*/
/*       Process until we get a request to change the status of       */
/*       the port.                                                    */
/*--------------------------------------------------------------------*/

  for (;;)
  {

#ifdef BIT32ENV
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

        case SUSPEND_QUERY:   /* query current status */

          nChar = (char) (suspend_processing ?
                              SUSPEND_WAITING : SUSPEND_ACTIVE );
          DosWrite(hPipe, &nChar, 1, &nBytes);

          break;

        case SUSPEND_SLEEP: /* suspend use of port and sleep */

          if ( suspend_processing ||
               interactive_processing ||
               terminate_processing )
          {
            nChar = SUSPEND_BUSY;
          }
          else {
            suspend_processing = KWTrue;

#ifdef BIT32ENV
            bDummyKill = KWTrue;
            DosKillProcess(DKP_PROCESS, getpid());
            nChar = (char) (DosWaitEventSem(semFree, 20000) ?
                     SUSPEND_ERROR : SUSPEND_OKAY);
            DosResetEventSem(semFree, &postCount);
#else
            DosFlagProcess(getpid(), FLGP_PID, PFLG_A, 0);
            nChar = (char) (DosSemSetWait(&semFree, 20000) ?
                     SUSPEND_ERROR : SUSPEND_OKAY);
#endif
          } /* else */

          DosWrite(hPipe, &nChar, 1, &nBytes);

          break;

        case SUSPEND_RESUME: /* Resume use of port */

          if ( !suspend_processing )
            nChar = SUSPEND_BUSY;
          else {
            suspend_processing = KWFalse;

#ifdef BIT32ENV
            DosPostEventSem(semWait);

            rc = DosResetEventSem(semReady, &postCount);
            if (rc)
              printOS2error( "DosResetEventSem", rc);

            rc = DosWaitEventSem(semReady, 20000);
            if (rc)
              printOS2error( "DosWaitEventSem", rc);

#else
            DosSemClear(&semWait);

            rc = DosSemSetWait(&semReady, 20000);
            if (rc)
              printOS2error( "DosSemSetWait", rc);
#endif
            nChar = SUSPEND_OKAY;

          } /* else */

          DosWrite(hPipe, &nChar, 1, &nBytes);

          break;

        default:

          nChar = SUSPEND_ERROR;
          DosWrite(hPipe, &nChar, 1, &nBytes);

          break;

      } /* switch */

    } /* for (;;) */

/*--------------------------------------------------------------------*/
/*         Drop the connection now we're done with this client.       */
/*--------------------------------------------------------------------*/

#ifdef BIT32ENV
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

#ifdef BIT32ENV

static void SuspendHandler(int nSig)
{
  if ( bDummyKill )
  {
    bDummyKill = KWFalse;
#if defined(__TURBOC__)
    signal(SIGTERM, (void (__cdecl *)(int))SuspendHandler);
#else
    signal(SIGTERM, SuspendHandler);
#endif

    raise(SIGUSR2);
  }
  else
  {
    signal(SIGTERM, SIG_DFL);
    DosKillProcess(DKP_PROCESS, getpid());
  }

} /* SuspendHandler */

#else

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

KWBoolean suspend_init(const char *port )
{

  char szPipe[FILENAME_MAX];
#ifndef BIT32ENV
  SEL selStack;
  PSZ pStack;
#endif
  TID tid;
  APIRET rc;

/*--------------------------------------------------------------------*/
/*      Set up the handler for signals from our suspend monitor       */
/*--------------------------------------------------------------------*/

#if defined(__TURBOC__)
  signal(SIGTERM, (void (__cdecl *)(int)) SuspendHandler);
#elif defined(BIT32ENV)
  signal(SIGTERM, SuspendHandler);
#else
  rc = DosSetSigHandler(SuspendHandler,
                        &old,
                        &nAction,
                        SIGA_ACCEPT,
                        SIG_PFLG_A);

  if (rc)
  {
    printOS2error( "DosSetSigHandler", rc);
    return KWFalse;
  }
#endif

/*--------------------------------------------------------------------*/
/*                Set up the pipe name to listen upon                 */
/*--------------------------------------------------------------------*/

  strcpy(szPipe, SUSPEND_PIPE);
  portName =  newstr( port );    /* Save for later reference           */
  strcat(szPipe, port );

  printmsg(4,"Creating locking pipe %s", szPipe );

#ifdef BIT32ENV
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
    return KWFalse;
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
    return KWFalse;
  }
#endif

#ifdef BIT32ENV

/*--------------------------------------------------------------------*/
/*       Now allocate the required semaphores.                        */
/*--------------------------------------------------------------------*/

   rc = DosCreateEventSem(NULL, &semFree, 0, 0);
   if (rc)
   {
      printOS2error( "DosCreateEventSem", rc);
      return KWFalse;
   }

   rc = DosCreateEventSem(NULL, &semWait, 0, 0);
   if (rc)
   {
      printOS2error( "DosCreateEventSem", rc);
      return KWFalse;
   }

   rc = DosCreateEventSem(NULL, &semReady, 0, 0);
   if (rc)
   {
      printOS2error( "DosCreateEventSem", rc);
      return KWFalse;
   }

#else

/*--------------------------------------------------------------------*/
/*       Now allocate memory for the monitor thread which will        */
/*       notify us if some program wants our port.                    */
/*--------------------------------------------------------------------*/

   rc = DosAllocSeg(STACKSIZE, &selStack, SEG_NONSHARED);

   if (rc)
   {
      printOS2error( "DosAllocSeg", rc);
      return KWFalse;
   }

   pStack = (PSZ) MAKEP(selStack, 0) + STACKSIZE -2 ;

#endif

/*--------------------------------------------------------------------*/
/*                    Now fire off the monitor thread                 */
/*--------------------------------------------------------------------*/

#ifdef BIT32ENV
   rc = DosCreateThread(&tid, SuspendThread, 0, 0, STACKSIZE);
#else
   rc = DosCreateThread(SuspendThread, &tid, pStack);
#endif

   if (rc)
   {
      printOS2error( "DosCreateThread", rc);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*                    Finally, our signal handler                     */
/*--------------------------------------------------------------------*/

#if defined(__TURBOC__)
   if ( signal( SIGUSR2, (void (__cdecl *)(int))usrhandler ) == SIG_ERR )
#else
   if ( signal( SIGUSR2, usrhandler ) == SIG_ERR )
#endif
   {
      printmsg( 0, "Couldn't set SIGUSR2\n" );
      panic();
   }

   return KWTrue;
} /* suspend_init */

/*--------------------------------------------------------------------*/
/*       s u s p e n d _ o t h e r                                    */
/*                                                                    */
/*       Request another UUCICO give up a modem                       */
/*--------------------------------------------------------------------*/

int suspend_other(const KWBoolean suspend,
                  const char *port )
{
  char szPipe[FILENAME_MAX];
  HFILE hPipe;
  U_INT nAction, nBytes;
  UCHAR nChar;
  APIRET rc = 1;
  KWBoolean firstPass = KWTrue;
  static KWBoolean suspended = KWFalse;
  int result;

  static time_t lastSuspend = 0;
  static char *lastPort = "";    /* Must not be be NULL pointer      */

  if ( ! suspend && ! suspended )
  {
     printmsg(4, "suspend_other: No port to resume.");
     return 0;
  }

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
           printOS2error( szPipe, rc);  /* So this is only for info  */

        if ((rc == ERROR_PIPE_BUSY) && firstPass )
        {
           firstPass = KWFalse;

#ifdef BIT32ENV
           rc = DosWaitNPipe( szPipe, 20000 ); /* Wait up to 20 sec for pipe  */
           if (rc)
           {
             printOS2error( "DosWaitNPipe", rc);
             return 0;
           } /* if (rc) */
#else
           rc = DosWaitNmPipe( szPipe, 20000 ); /* Wait up to 20 sec for pipe  */
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

   nChar = (UCHAR) (suspend ? SUSPEND_SLEEP : SUSPEND_RESUME);
   rc = DosWrite(hPipe, &nChar, 1, &nBytes);

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
   else if ( nChar != SUSPEND_OKAY )
   {
     printmsg(0, "Cannot %s background uucico on port %s.  Result code was %c",
                 suspend ? "suspend" : "resume",
                 port,
                 nChar );
     result = -3;
   }
   else {
      result = 1;                   /* Success!                       */
      suspended = suspend;          /* Remember for future            */
   }

/*--------------------------------------------------------------------*/
/*                     Close up and return to caller                  */
/*--------------------------------------------------------------------*/

  DosClose(hPipe);

  if (!suspend)
    DosSleep(3000); /* apparently we need to give the other one some time! */

  return result;

} /* suspend_other */

/*--------------------------------------------------------------------*/
/*       s u s p e n d _ w a i t                                      */
/*                                                                    */
/*       Wait to take the serial port back                            */
/*--------------------------------------------------------------------*/

CONN_STATE suspend_wait(void)
{
   APIRET rc;

   printmsg(0,"suspend_wait: Port %s released, program sleeping",
               portName );

#ifdef BIT32ENV
   rc = DosPostEventSem(semFree);
   if (rc)
      printOS2error( "DosPostEventSem", rc);

   rc = DosWaitEventSem(semWait, SEM_INDEFINITE_WAIT);
   if (rc)
      printOS2error( "DosWaitEventSem", rc);

   rc = DosResetEventSem(semWait, &postCount);
   if (rc)
      printOS2error( "DosResetEventSem", rc);

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

} /* suspend_wait */

/*--------------------------------------------------------------------*/
/*       s u s p e n d _ r e a d y                                    */
/*                                                                    */
/*       We are ready to release for the next time                    */
/*--------------------------------------------------------------------*/

void suspend_ready(void)
{
   APIRET rc;

   printmsg(2,"suspend_ready: Port %s in use", portName );

#ifdef BIT32ENV
   rc = DosPostEventSem(semReady);
   if (rc)
      printOS2error( "DosPostEventSem", rc);
#else
   rc = DosSemClear(&semReady);
   if (rc)
      printOS2error( "DosSemClear", rc);
#endif
} /* suspend_wait */
