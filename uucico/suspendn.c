/*--------------------------------------------------------------------*/
/*    s u s p e n d n . c                                             */
/*                                                                    */
/*    suspend/resume uupoll/uucico daemon (for NT)                    */
/*                                                                    */
/*    Author: David Watt and Kai Uwe Rommel                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Copyright (c) 1993 by Kai Uwe Rommel                         */
/*       Changes copyright (c) 1993 by David Watt                     */
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
 *    $Id: suspendn.c 1.15 2001/03/12 13:54:49 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: suspendn.c $
 *    Revision 1.15  2001/03/12 13:54:49  ahd
 *    Annual copyright update
 *
 *    Revision 1.14  2000/05/12 12:32:00  ahd
 *    Annual copyright update
 *
 *    Revision 1.13  1999/01/08 02:20:56  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.12  1999/01/04 03:53:30  ahd
 *    Annual copyright change
 *
 *    Revision 1.11  1998/03/01 01:40:26  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1997/04/24 01:35:14  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1996/01/01 21:22:37  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1995/01/07 16:40:02  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.7  1994/12/22 00:36:20  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1994/12/09 03:50:40  rommel
 *    Correct errors with back-to-back uses of port failing
 *
 *        Revision 1.5  1994/10/23  23:29:44  ahd
 *        Better control of suspension of processing
 *
 *        Revision 1.4  1994/05/01  21:59:06  dmwatt
 *        Trap errors from failure of suspend_init to create pipe
 *
 * Revision 1.3  1994/02/19  05:10:36  ahd
 * Use standard first header
 *
 * Revision 1.2  1994/01/01  19:21:10  ahd
 * Annual Copyright Update
 *
 * Revision 1.1  1993/12/29  03:34:37  dmwatt
 * Initial revision
 *
 *
 * Revision 1.1  1993/09/27  00:45:20  dmwatt
 * Initial revision. Split from suspend2.c.
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

#include "uupcmoah.h"

#include <windows.h>
#include <limits.h>
#include <signal.h>
#include <process.h>

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
#include "pnterr.h"
#include "suspend.h"
#include "ssleep.h"

#define STACKSIZE 8192

KWBoolean suspend_processing = KWFalse;

static HANDLE hPipe;
static char nChar;
static char *portName;

static HANDLE eventWait = INVALID_HANDLE_VALUE;
static HANDLE eventFree = INVALID_HANDLE_VALUE;
static ULONG postCount;
static ULONG nBytes;

RCSID("$Id: suspendn.c 1.15 2001/03/12 13:54:49 ahd v1-13k $");

BOOL AbortComm(void);

/*--------------------------------------------------------------------*/
/*       S u s p e n d T h r e a d                                    */
/*                                                                    */
/*       Accept request to release serial port                        */
/*--------------------------------------------------------------------*/

DWORD WINAPI SuspThread(LPVOID *ignored)
{
   DWORD dwRet;

/*--------------------------------------------------------------------*/
/*       Process until we get a request to change the status of       */
/*       the port.                                                    */
/*--------------------------------------------------------------------*/

  for (;;)
  {
    if ( !ConnectNamedPipe(hPipe, NULL) ) {
       break;
    }

    for (;;)
    {
      if ( !ReadFile(hPipe, &nChar, 1, &nBytes, NULL) ) {
         break;                   /* Quit if an error                  */
      }

      if ( nBytes == 0 )
        break; /* EOF */

/*--------------------------------------------------------------------*/
/*               Handle the received command character                */
/*--------------------------------------------------------------------*/

      switch ( nChar )
      {

        case SUSPEND_QUERY: /* query */

          nChar = (char) (suspend_processing ? SUSPEND_SLEEP : SUSPEND_RESUME);
          WriteFile(hPipe, &nChar, 1, &nBytes, NULL);

          break;

        case SUSPEND_SLEEP: /* suspend */

          if ( suspend_processing ||
               interactive_processing ||
               terminate_processing )
          {
            nChar = SUSPEND_ERROR;
          }
          else {
            BOOL retval;

            suspend_processing = KWTrue;
            retval = AbortComm();

            if (!retval)
            {
               printNTerror("AbortComm", GetLastError());
            }

            dwRet = WaitForSingleObject(eventFree, 20000);

            if (WAIT_FAILED == dwRet || WAIT_TIMEOUT == dwRet)
            {
               printNTerror("WaitForSingleObject eventFree", GetLastError());
               nChar = SUSPEND_ERROR;
            }
            else if (WAIT_OBJECT_0 == dwRet)
            { /* It worked, event is set */
               nChar = SUSPEND_OKAY;
            } else
            {
               printmsg(0, "WaitForSingleObject: strange return %u", dwRet);
            }
          } /* else */

          WriteFile(hPipe, &nChar, 1, &nBytes, NULL);

          break;

        case SUSPEND_RESUME: /* release */

          if ( !suspend_processing )
            nChar = SUSPEND_ERROR;
          else {
            suspend_processing = KWFalse;

            SetEvent(eventWait);
            nChar = SUSPEND_OKAY;

          } /* else */

          WriteFile(hPipe, &nChar, 1, &nBytes, NULL);

          break;

        default:

          nChar = SUSPEND_ERROR;
          WriteFile(hPipe, &nChar, 1, &nBytes, NULL);

          break;

      } /* switch */

    } /* for (;;) */

/*--------------------------------------------------------------------*/
/*         Drop the connection now we're done with this client.       */
/*--------------------------------------------------------------------*/
     DisconnectNamedPipe(hPipe);

  } /* for (;;) */

  printmsg(5, "SuspThread: exiting suspend thread.");
  ExitThread(0);

  return 0;

} /* SuspThread */

/*--------------------------------------------------------------------*/
/*       s u s p e n d _ i n i t                                      */
/*                                                                    */
/*       Initialize thread to handle port suspension                  */
/*--------------------------------------------------------------------*/

KWBoolean suspend_init(const char *port )
{
  char szPipe[FILENAME_MAX];
  DWORD tid;
  HANDLE hThread;

/*--------------------------------------------------------------------*/
/*                Set up the pipe name to listen upon                 */
/*--------------------------------------------------------------------*/

  strcpy(szPipe, SUSPEND_LOCAL SUSPEND_PIPE);
  portName =  newstr( port );    /* Save for later reference           */
  strcat(szPipe, port );

  printmsg(4,"Creating blocking pipe %s", szPipe );

  hPipe = CreateNamedPipe( szPipe,
                     PIPE_ACCESS_DUPLEX | FILE_FLAG_WRITE_THROUGH,
                     PIPE_WAIT | PIPE_READMODE_BYTE | PIPE_TYPE_BYTE,
                     1,  /* One instances of the pipe permitted */
                     32, /* out buffer size in bytes */
                     32, /* input buffer size in bytes */
                     5000, /* Timeout time in milliseconds */
                     NULL); /* No security attributes */

  if (INVALID_HANDLE_VALUE == hPipe)
  {
    printNTerror( "CreateNamedPipe", GetLastError());
    return KWFalse;
  }

/*--------------------------------------------------------------------*/
/*                    Now create the events that pass the word        */
/*--------------------------------------------------------------------*/

   eventWait = CreateEvent(NULL, KWFalse, FALSE, NULL);
   if (eventWait == INVALID_HANDLE_VALUE) {
      printNTerror("CreateEvent", GetLastError());
      return KWFalse;
   }

   eventFree = CreateEvent(NULL, KWFalse, FALSE, NULL);
   if (eventFree == INVALID_HANDLE_VALUE) {
      printNTerror("CreateEvent", GetLastError());
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*                    Now fire off the monitor thread                 */
/*--------------------------------------------------------------------*/

   hThread = CreateThread(NULL, 8192, SuspThread, NULL, 0, &tid);

   if (INVALID_HANDLE_VALUE == hThread)
   {
      printNTerror( "CreateThread", GetLastError());
      return KWFalse;
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
  HANDLE hPipe = INVALID_HANDLE_VALUE;
  DWORD nBytes;
  UCHAR nChar;
  BOOL rc;
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

  strcpy(szPipe, SUSPEND_LOCAL SUSPEND_PIPE);
  strcat(szPipe, port );

/*--------------------------------------------------------------------*/
/*           Try to open the pipe, with one retry if needed           */
/*--------------------------------------------------------------------*/

  while (INVALID_HANDLE_VALUE == hPipe)
  {
      hPipe = CreateFile(szPipe,
                   GENERIC_READ | GENERIC_WRITE,
                   0,
                   NULL,
                   OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL,
                   NULL);

      if (INVALID_HANDLE_VALUE == hPipe)
      {
        DWORD dwErr = GetLastError();

        if ( debuglevel >= 4 )          /* No error if no passive UUCICO  */
           printNTerror(szPipe, dwErr);  /* So this is only for info  */

        if ((dwErr == ERROR_PIPE_BUSY) && firstPass )
        {
           firstPass = KWFalse;

           rc = WaitNamedPipe(szPipe, 5000); /* Wait up to 5 sec for pipe  */
           if (!rc)
           {
             printNTerror( "WaitNamedPipe", GetLastError());
             return 0;
           } /* if (!rc) */
        } /* if */
        else
           return 0;

      } /* if */

   } /* while (INVALID_HANDLE_VALUE == hPipe) */

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

   rc = WriteFile(hPipe, suspend ? "S" : "R", 1, &nBytes, NULL);
   if (!rc)
   {
     printNTerror( "WriteFile", GetLastError());
     CloseHandle(hPipe);
     return -1;
   }

   if ( nBytes != 1 )
   {
     CloseHandle(hPipe);
     return -1;
   }

   printmsg(2, "Waiting for background uucico to %s use of %s ...",
               suspend ? "suspend" : "resume",
               port);

/*--------------------------------------------------------------------*/
/*                      Process the server response                   */
/*--------------------------------------------------------------------*/

   rc = ReadFile(hPipe, &nChar, 1, &nBytes, NULL);

   if (!rc)
   {
     printNTerror( "ReadFile", GetLastError());
     result = -1;
   }
   else if ( nBytes != 1 )
   {
     printmsg(0,"suspend_other: Error: No data from remote UUCICO");
     result = -2;
   }
   else if ( nChar != SUSPEND_OKAY )
   {
     printmsg(0, "Cannot %s background uucico.  Result code was %c",
                 suspend ? "suspend" : "resume",
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

  CloseHandle(hPipe);

  return result;

} /* suspend_other */

/*--------------------------------------------------------------------*/
/*       s u s p e n d _ w a i t                                      */
/*                                                                    */
/*       Wait to take the serial port back                            */
/*--------------------------------------------------------------------*/

CONN_STATE suspend_wait(void)
{
   DWORD rc;
   BOOL retval;

   printmsg(0,"suspend_wait: Port %s released, program sleeping",
               portName );

   retval = SetEvent(eventFree);

   if (!retval)
      printNTerror( "SetEvent", GetLastError());

   rc = WaitForSingleObject(eventWait, INFINITE);

   if (WAIT_FAILED == rc) {
      printNTerror( "WaitForSingleObject", GetLastError());
      return CONN_EXIT;
   } else {
      return CONN_INITIALIZE;
   }

} /* suspend_wait */

/*--------------------------------------------------------------------*/
/*       s u s p e n d _ r e a d y                                    */
/*                                                                    */
/*       Notify pipe we are ready to suspend                          */
/*--------------------------------------------------------------------*/

void suspend_ready(void)
{
   // No operation under Windows NT for now
} /* suspend_ready */
