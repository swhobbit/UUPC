/*--------------------------------------------------------------------*/
/*    s u s p e n d 2 . c                                             */
/*                                                                    */
/*    suspend/resume uupoll/uucico daemon (for OS/2)                  */
/*                                                                    */
/*    Author: Kai Uwe Rommel                                          */
/*                                                                    */
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
 *    $Id: lib.h 1.12 1993/09/20 04:51:31 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <signal.h>
#include <process.h>

#define INCL_DOS
#define INCL_DOSPROCESS
#define INCL_DOSSIGNALS
#include <os2.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "dcp.h"
#include "dcpsys.h"
#include "modem.h"
#include "catcher.h"

#ifdef __OS2__
#include "pos2err.h"
#endif

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

#ifdef __OS2__

static HEV semWait, semFree;
static ULONG postCount;
static ULONG nBytes;

#else

static ULONG semWait, semFree;
static PFNSIGHANDLER old;
static USHORT nAction;
static int nBytes;
typedef USHORT APIRET ;  // Define older API return type

#endif


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

static VOID FAR SuspendThread(VOID)
{

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
        break;
      if ( nBytes == 0 )
        break; /* EOF */

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
          nChar = 'E';
        else
        {
          suspend_processing = TRUE;

#ifdef __OS2__
          raise(SIGUSR1);
#else
          DosFlagProcess(getpid(), FLGP_PID, PFLG_A, 0);
#endif

#ifdef __OS2__
          nChar = (char) (DosWaitEventSem(&semFree, 20000) ? 'T' : 'O');
#else
          nChar = (char) (DosSemSetWait(&semFree, 20000) ? 'T' : 'O');
#endif
        }

        DosWrite(hPipe, &nChar, 1, &nBytes);

        break;

      case 'R': /* release */

        if ( !suspend_processing )
          nChar = 'E';
        else
        {
          suspend_processing = FALSE;

#ifdef __OS2__
          DosResetEventSem( &semWait, &postCount );
#else
          DosSemClear(&semWait);
#endif
          nChar = 'O';
        }

        DosWrite(hPipe, &nChar, 1, &nBytes);

        break;

      default:

        nChar = 'U';
        DosWrite(hPipe, &nChar, 1, &nBytes);

        break;

      }
    }

#ifdef __OS2__
    DosDisConnectNPipe(hPipe);
#else
    DosDisConnectNmPipe(hPipe);
#endif
  }

  DosExit(EXIT_THREAD, 0);

} /* SuspendThread */

#ifndef __OS2__
/*--------------------------------------------------------------------*/
/*       S u s p e n d H a n d l e r                                  */
/*                                                                    */
/*       Signal handler for suspend hander                            */
/*--------------------------------------------------------------------*/

static VOID FAR PASCAL SuspendHandler(USHORT nArg, USHORT nSig)
{

  DosSetSigHandler(SuspendHandler, &old, &nAction,
                   SIGA_ACKNOWLEDGE, SIG_PFLG_A);
  raise(SIGINT);

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

void suspend_init(void)
{
  char szPipe[64];
  SEL selStack;
  PSZ pStack;
  TID tid;

#ifndef __OS2__
  if ( DosSetSigHandler(SuspendHandler, &old, &nAction,
                        SIGA_ACCEPT, SIG_PFLG_A) )
    return;
#endif

  strcpy(szPipe, "\\PIPE\\UUCICO\\");
  strcat(szPipe, E_inmodem);

#ifdef __OS2__
  if ( DosCreateNPipe( szPipe,
                     &hPipe,
                     NP_ACCESS_DUPLEX | NP_NOINHERIT | NP_NOWRITEBEHIND,
                     NP_WAIT | NP_READMODE_BYTE | NP_TYPE_BYTE | 1,
                     32,
                     32,
                     5000) )
    return;
#lse
  if ( DosMakeNmPipe(szPipe,
                     &hPipe,
                     NP_ACCESS_DUPLEX | NP_NOINHERIT | NP_NOWRITEBEHIND,
                     NP_WAIT | NP_READMODE_BYTE | NP_TYPE_BYTE | 1,
                     32,
                     32,
                     5000) )
    return;
#endif

#ifdef __OS2__
  pStack = malloc( STACKSIZE );
#else
  if ( DosAllocSeg(STACKSIZE, &selStack, SEG_NONSHARED) )
    return;

  pStack = (PSZ) MAKEP(selStack, 0) + STACKSIZE -2 ;
#endif

  DosCreateThread(SuspendThread, &tid, pStack);
} /* suspend_init */

/*--------------------------------------------------------------------*/
/*       s u s p e n d _ o t h e r                                    */
/*                                                                    */
/*       Request another UUCICO give up a modem                       */
/*--------------------------------------------------------------------*/

int suspend_other(boolean suspend)
{
  char szPipe[64];
  HFILE hPipe;
  USHORT nAction, nBytes;
  UCHAR nChar;

  strcpy(szPipe, "\\PIPE\\UUCICO\\");
  strcat(szPipe, flds[FLD_TYPE]);

  if ( DosOpen(szPipe,
               &hPipe,
               &nAction,
               0,
               0,
               FILE_OPEN,
               OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
               0) )
    return 0;

  if ( DosWrite(hPipe, suspend ? "S" : "R", 1, &nBytes) || nBytes != 1 )
    return DosClose(hPipe), -1;

  printmsg(1, "Waiting for background uucico to %s ...",
           suspend ? "suspend" : "resume");

  if ( DosRead(hPipe, &nChar, 1, &nBytes) || nBytes != 1 )
    return DosClose(hPipe), -1;

  if ( nChar != 'O' )
  {
    printmsg(0, "Cannot %s background uucico.",
             suspend ? "suspend" : "resume");
    return -2;
  }

  DosClose(hPipe);

  return 1;
} /* suspend_other */

/*--------------------------------------------------------------------*/
/*       s u s p e n d _ w a i t                                      */
/*                                                                    */
/*       Wait to take the serial port back                            */
/*--------------------------------------------------------------------*/

CONN_STATE suspend_wait(void)
{
   APIRET rc;

#ifdef __OS2__
   DosResetEventSem( &semFree, &postCount);

   rc = DosWaitEventSem(&semWait, SEM_INDEFINITE_WAIT);
#else
   DosSemClear(&semFree);

   rc = DosSemSetWait(&semWait, SEM_INDEFINITE_WAIT);
#endif

   if ( rc )
   {

#ifdef __OS2__
      printOS2error( "DosWaitEventSem", rc );
#endif
      return CONN_EXIT;
   }
   else
      return CONN_INITIALIZE;

} /* suspend_wait */
