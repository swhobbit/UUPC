/*--------------------------------------------------------------------*/
/*       u l i b i p . c                                              */
/*                                                                    */
/*       NetBIOS communications driver for DOS                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2001 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ulibnb.c 1.10 2000/05/12 12:32:00 ahd v1-13g $
 *
 *    $Log: ulibnb.c $
 *    Revision 1.10  2000/05/12 12:32:00  ahd
 *    Annual copyright update
 *
 *    Revision 1.9  1999/01/08 02:20:56  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.8  1999/01/04 03:53:30  ahd
 *    Annual copyright change
 *
 *    Revision 1.7  1998/03/01 01:40:46  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1997/04/24 01:35:50  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1996/01/01 21:23:03  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1995/03/24 04:17:22  ahd
 *    Compiler warning message cleanup, optimize for low memory processing
 *
 *    Revision 1.3  1995/01/07 16:40:38  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.2  1994/12/22 00:37:14  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.1  1994/03/28 02:18:45  ahd
 *    Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "ulibnb.h"
#include "comm.h"          /* Modem status bits                       */
#include "ssleep.h"
#include "catcher.h"

#include "commlib.h"       /* Trace functions, etc.                    */

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

KWBoolean IsFatalSocketError(int err);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

RCSID("$Id: ulibnb.c 1.10 2000/05/12 12:32:00 ahd v1-13g $");
static KWBoolean hangupNeeded = KWTrue;
extern KWBoolean winsockActive;                  /* Initialized in catcher.c  */
static SOCKET pollingSock = INVALID_SOCKET;     /* The current polling socket  */
static SOCKET connectedSock = INVALID_SOCKET;   /* The currently connected socket  */

static KWBoolean connectionDied = KWFalse;        /* The current connection failed  */

/*--------------------------------------------------------------------*/
/*                           Local defines                            */
/*--------------------------------------------------------------------*/

#ifndef MAKEWORD
#define MAKEWORD(a, b) ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#endif

/*--------------------------------------------------------------------*/
/*    b o p e n a c t i v e                                           */
/*                                                                    */
/*    Open an active socket connection for I/O                        */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

int bactiveopenline(char *name, BPS bps, const KWBoolean direct)
{
   SOCKADDR_IN sin;
   LPHOSTENT phe;
   LPSERVENT pse;
   u_short remotePort;
   char *portStr;

   if (portActive)              /* Was the port already active?      */
      closeline();               /* Yes --> Shutdown it before open  */

   printmsg(15, "bactiveopenline: %s", name);

   norecovery = KWFalse;    /* Flag we need a graceful shutdown after  */
                           /* Ctrl-BREAK                              */

   carrierDetect = KWFalse;  /* No modem connected yet               */

   connectionDied = KWFalse; /* The connection hasn't failed yet */

/*--------------------------------------------------------------------*/
/*            Create add ourself to the NETBIOS name table            */
/*--------------------------------------------------------------------*/

   if (NetbiosAddName(Name) == ILLEGAL_NAME_NUM)
   {
      printmsg(0, "tactiveopenline: NetbiosAddName failed");
      return KWTrue;
   }

   if (!ControlNcb.NcbRetCode)
   {
      printmsg(4,"bactiveopenline: session successfully created");
      SessionLsn = ControlNcb.NcbLsn;
      return SUCCESS;
   } else {
      printmsg(0,"bactiveopenline: session not created...error 0x%02X",
                                     ControlNcb.NcbRetCode);
      return FAILURE;
   }

   traceStart( name );

   portActive = KWTrue;    /* record status for error handler */
   carrierDetect = KWTrue;  /* Carrier detect = connection              */

   return KWFalse;                      /* Return success to caller     */

} /* bactiveopenline */

/*--------------------------------------------------------------------*/
/*    b o p e n p a s s i v e                                         */
/*                                                                    */
/*    Listen on a socket for an incoming uucp connection              */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

int tpassiveopenline(char *name, BPS bps, const KWBoolean direct)
{

   if (portActive)              /* Was the port already active?      */
      closeline();               /* Yes --> Shutdown it before open  */

   printmsg(15, "tpassiveopenline: opening passive connection");

   norecovery = KWFalse;    /* Flag we need a graceful shutdown after  */
                           /* Ctrl-BREAK                              */

   carrierDetect = KWFalse;  /* No network connection yet            */

   connectionDied = KWFalse; /* The connection hasn't failed yet */

   traceStart( name );

   portActive = KWTrue;    /* record status for error handler */

   return KWFalse;         /* Return success to caller                 */

} /* tpassiveopen */

/*--------------------------------------------------------------------*/
/*    b s r e a d                                                     */
/*                                                                    */
/*    Read from the network socket                                    */
/*                                                                    */
/*    Non-blocking read essential to "g" protocol.  See               */
/*    "dcpgpkt.c" for description.  This all changes in a             */
/*    multi-tasking system.  Requests for I/O should get queued       */
/*    and an event flag given.  Then the requesting process (e.g.     */
/*    gmachine()) waits for the event flag to fire processing         */
/*    either a read or a write.  Could be implemented on VAX/VMS      */
/*    or DG but not MS-DOS.                                           */
/*--------------------------------------------------------------------*/

unsigned int tsread(char UUFAR *output,
                    unsigned int wanted,
                    unsigned int timeout)
{
   time_t stop_time ;
   time_t now ;
   int nReady;
   fd_set readfds;
   struct timeval tm;

   KWBoolean firstPass = KWTrue;

/*--------------------------------------------------------------------*/
/*                           Validate input                           */
/*--------------------------------------------------------------------*/

   if ( wanted > commBufferLength )
   {
      printmsg(0,"tsread: Overlength read, wanted %u bytes into %u buffer!",
                     (unsigned int) wanted,
                     (unsigned int) commBufferLength );
      panic();
   }

/*--------------------------------------------------------------------*/
/*           Determine if our internal buffer has the data            */
/*--------------------------------------------------------------------*/

   if (commBufferUsed >= wanted)
   {
      MEMCPY( output, commBuffer, wanted );
      commBufferUsed -= wanted;
      if ( commBufferUsed )   /* Any data left over?                 */
         MEMMOVE( commBuffer, commBuffer + wanted, commBufferUsed );
                              /* Yes --> Save it                     */
      return wanted + commBufferUsed;
   } /* if */

   if (connectionDied || (connectedSock == INVALID_SOCKET) )
   {                             /* Haven't accepted a connection yet?  */
      return 0;
   }

/*--------------------------------------------------------------------*/
/*                 Determine when to stop processing                  */
/*--------------------------------------------------------------------*/

   if ( timeout == 0 )
   {
      stop_time = 0;
      now = 1;                /* Any number greater than stop time   */
   }
   else {
      time( & now );
      stop_time = now + timeout;
   }

   do {
      int received;
      int needed = wanted - commBufferUsed;

/*--------------------------------------------------------------------*/
/*          Initialize fd_set structure for select() call             */
/*--------------------------------------------------------------------*/

      FD_ZERO(&readfds);
      FD_SET(connectedSock, &readfds);

/*--------------------------------------------------------------------*/
/*                     Handle an aborted program                      */
/*--------------------------------------------------------------------*/

      if ( terminate_processing )
      {
         static KWBoolean recurse = KWFalse;
         if ( ! recurse )
         {
            printmsg(2,"tsread: User aborted processing");
            recurse = KWTrue;
         }
         return 0;
      }

/*--------------------------------------------------------------------*/
/*       Special case for network sockets: block for at least 5      */
/*       msec if we have to read at least one character (this         */
/*       needs to be tuned)                                           */
/*--------------------------------------------------------------------*/
      if ( stop_time <= now )
      {
         tm.tv_usec = 5000;
         tm.tv_sec = 0;
      }
      else {
         tm.tv_sec = stop_time - now;
         tm.tv_usec = 0;

      }

/*--------------------------------------------------------------------*/
/*                 Read the data from the socket                      */
/*--------------------------------------------------------------------*/

      nReady = select(1, &readfds, NULL, NULL, &tm);
      if (nReady == SOCKET_ERROR)
      {
         int err = WSAGetLastError();
         printmsg(0, "tsread: error in select()");
         printWSerror("select", err);

         if (IsFatalSocketError(err))
         {
            shutdown(connectedSock, 2);  /* Fail both reads and writes  */
            connectionDied = KWTrue;
         }
         commBufferUsed = 0;
         return 0;
      }
      else if (nReady == 0)
      {
         printmsg(5, "tsread: timeout after %d seconds",timeout);
         commBufferUsed = 0;
         return 0;
      }
      else {
         received = recv(connectedSock,
                         commBuffer + commBufferUsed,
                         firstPass ?
                            commBufferLength - commBufferUsed : needed,
                         0);

         firstPass = KWFalse;

         if (received == SOCKET_ERROR)
         {
            int wsErr = WSAGetLastError();

            printmsg(0, "tsread: recv() failed");
            printWSerror("recv", wsErr);
            commBufferUsed = 0;
            return 0;
         }
      }  /* else */

#ifdef UDEBUG
      printmsg(15,"sread: Want %d characters, received %d, total %d in buffer",
                  (int) wanted,
                  (int) received,
                  (int) commBufferUsed + received);
#endif

/*--------------------------------------------------------------------*/
/*                    Log the newly received data                     */
/*--------------------------------------------------------------------*/

      traceData( commBuffer + commBufferUsed,
                 (unsigned) received,
                 KWFalse );

/*--------------------------------------------------------------------*/
/*            If we got the data, return it to the caller             */
/*--------------------------------------------------------------------*/

      commBufferUsed += received;
      if ( commBufferUsed >= wanted )
      {
         MEMCPY( output, commBuffer, wanted );
         commBufferUsed -= wanted;
         if ( commBufferUsed )   /* Any data left over?              */
            MEMMOVE( commBuffer, commBuffer + wanted, commBufferUsed );

         return wanted;

      } /* if */

/*--------------------------------------------------------------------*/
/*                 Update the clock for the next pass                 */
/*--------------------------------------------------------------------*/

      if (stop_time > 0)
         time( &now );

   } while (stop_time > now);

/*--------------------------------------------------------------------*/
/*         We don't have enough data; report what we do have          */
/*--------------------------------------------------------------------*/

   return commBufferUsed;

} /* tsread */

/*--------------------------------------------------------------------*/
/*    b s w r i t e                                                   */
/*                                                                    */
/*    Write to the open socket                                        */
/*   Note:  this is non-blocking, so we've got to use select() to     */
/*    gradually write out the entire buffer                           */
/*--------------------------------------------------------------------*/

int tswrite(const char UUFAR *data, unsigned int len)
{
   int status;

/* Has connection died? */
   if (connectionDied || connectedSock == INVALID_SOCKET)
      return 0;

   status = send(connectedSock, data, len, 0);

   if (status == SOCKET_ERROR)
   {
      int err;

      err = WSAGetLastError();
      printmsg(0, "tswrite: Error sending data to socket");
      printWSerror("send", err);

      if (IsFatalSocketError(err))
      {
         shutdown(connectedSock, 2);  /* Fail both reads and writes   */
         connectionDied = KWTrue;
      }
      return 0;
   }

   if (status < (int)len)     /* Breaks if len > 32K, which is unlikely */
   {
      printmsg(0,"tswrite: Write to network failed.");
      return status;
   }

/*--------------------------------------------------------------------*/
/*                        Log the data written                        */
/*--------------------------------------------------------------------*/

   traceData( data, len, KWTrue );

/*--------------------------------------------------------------------*/
/*              Return byte count transmitted to caller               */
/*--------------------------------------------------------------------*/

   return len;

} /* tswrite */

/*--------------------------------------------------------------------*/
/*    b s s e n d b r k                                               */
/*                                                                    */
/*    Send a break signal over the network                            */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

void tssendbrk(unsigned int duration)
{
   printmsg(12, "tsendbrk: ignored break of duration %d", duration);
   return;

} /* tssendbrk */

/*--------------------------------------------------------------------*/
/*    b c l o s e l i n e                                             */
/*                                                                    */
/*    Close the serial port down                                      */
/*--------------------------------------------------------------------*/

void tcloseline(void)
{
   if (!portActive)
      panic();

   portActive = KWFalse;    /* flag port closed for error handler  */

   if (connectedSock != INVALID_SOCKET)
   {
      closesocket(connectedSock);
      connectedSock = INVALID_SOCKET;
   }

   if (pollingSock != INVALID_SOCKET)
   {
      closesocket(pollingSock);
      pollingSock = INVALID_SOCKET;
   }

   carrierDetect = KWFalse;  /* No network connection yet              */
   traceStop();

} /* tcloseline */

/*--------------------------------------------------------------------*/
/*    b h a n g u p                                                   */
/*                                                                    */
/*    Break the connection with the remote system.                    */
/*--------------------------------------------------------------------*/

void thangup( void )
{
   if (!hangupNeeded)
      return;
   hangupNeeded = KWFalse;

   if (connectedSock != INVALID_SOCKET)
   {
      closesocket(connectedSock);
      connectedSock = INVALID_SOCKET;
   }

   if (pollingSock != INVALID_SOCKET)
   {
      closesocket(pollingSock);
      pollingSock = INVALID_SOCKET;
   }

   carrierDetect = KWFalse;  /* No network connection yet              */

} /* thangup */

/*--------------------------------------------------------------------*/
/*    b S I O S p e e d                                               */
/*                                                                    */
/*    Re-specify the speed of an opened serial port                   */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

void tSIOSpeed(BPS bps)
{
   return;  /* Irrelevant on network */
} /* iSIOSpeed */

/*--------------------------------------------------------------------*/
/*    b f l o w c o n t r o l                                         */
/*                                                                    */
/*    Enable/Disable in band (XON/XOFF) flow control                  */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

void tflowcontrol( KWBoolean flow )
{
   return;                  /* Irrelevant on network (we hope)       */
} /* tflowcontrol */

/*--------------------------------------------------------------------*/
/*    b G e t S p e e d                                               */
/*                                                                    */
/*    Report current speed of communications connection               */
/*--------------------------------------------------------------------*/

BPS tGetSpeed( void )
{
   return 57600;           /* Arbitary large number to avoid possible  */
                           /* divide by zero error in caller           */
} /* GetSpeed */

/*--------------------------------------------------------------------*/
/*    b C D                                                           */
/*                                                                    */
/*    Report if we have carrier detect and lost it                    */
/*--------------------------------------------------------------------*/

KWBoolean tCD( void )
{
   KWBoolean online = carrierDetect;

   return online;
} /* tCD */

/*--------------------------------------------------------------------*/
/*       b W a i t F o r N e t C o n n e c t                          */
/*                                                                    */
/*       Wait for remote system to connect to our waiting server      */
/*--------------------------------------------------------------------*/

KWBoolean tWaitForNetConnect(const unsigned int timeout)
{
   fd_set readfds;
   int nReady;
   struct timeval tm;

   tm.tv_sec = timeout;
   tm.tv_usec = 0;

   FD_ZERO(&readfds);
   FD_SET(pollingSock, &readfds);

   nReady = select(1, &readfds, NULL, NULL, &tm);

   if (nReady == SOCKET_ERROR)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "WaitForNetConnect: select() failed");
      printWSerror("select", wsErr);
      return KWFalse;
   }
   else if (nReady == 0)
   {
      printmsg(5, "WaitForNetConnect: select() timed out");
      return KWFalse;
   }

   if (terminate_processing)
      return KWFalse;

   connectedSock = accept(pollingSock, NULL, NULL);
   if (connectedSock == INVALID_SOCKET)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "WaitForNetConnect: could not accept a connection");
      printWSerror("accept", wsErr);
   }

   carrierDetect = KWTrue;

   return KWTrue;

} /* tWaitForNetConnect */
