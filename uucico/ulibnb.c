/*--------------------------------------------------------------------*/
/*       u l i b i p . c                                              */
/*                                                                    */
/*       NetBIOS communications driver for DOS                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1994 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ulibip.c 1.17 1994/02/19 05:11:47 ahd Exp $
 *
 *    $Log: ulibip.c $
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

boolean IsFatalSocketError(int err);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

static boolean carrierDetect = FALSE;

currentfile();
static boolean hangupNeeded = TRUE;
extern boolean winsockActive;                   /* Initialized in catcher.c  */
static SOCKET pollingSock = INVALID_SOCKET;     /* The current polling socket  */
static SOCKET connectedSock = INVALID_SOCKET;   /* The currently connected socket  */

static boolean connectionDied = FALSE;          /* The current connection failed  */

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

int bactiveopenline(char *name, BPS bps, const boolean direct)
{
   SOCKADDR_IN sin;
   LPHOSTENT phe;
   LPSERVENT pse;
   u_short remotePort;
   char *portStr;

   if (portActive)              /* Was the port already active?      */
      closeline();               /* Yes --> Shutdown it before open  */

   printmsg(15, "bactiveopenline: %s", name);

   norecovery = FALSE;     /* Flag we need a graceful shutdown after  */
                           /* Ctrl-BREAK                              */

   carrierDetect = FALSE;  /* No modem connected yet                */

   connectionDied = FALSE; /* The connection hasn't failed yet */

/*--------------------------------------------------------------------*/
/*            Create add ourself to the NETBIOS name table            */
/*--------------------------------------------------------------------*/

   if (NetbiosAddName(Name) == ILLEGAL_NAME_NUM)
   {
      printmsg(0, "tactiveopenline: NetbiosAddName failed");
      return TRUE;
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

   portActive = TRUE;     /* record status for error handler */
   carrierDetect = TRUE;   /* Carrier detect = connection              */

   return FALSE;                       /* Return success to caller     */

} /* bactiveopenline */

/*--------------------------------------------------------------------*/
/*    b o p e n p a s s i v e                                         */
/*                                                                    */
/*    Listen on a socket for an incoming uucp connection              */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

int tpassiveopenline(char *name, BPS bps, const boolean direct)
{

   if (portActive)              /* Was the port already active?      */
      closeline();               /* Yes --> Shutdown it before open  */

   printmsg(15, "tpassiveopenline: opening passive connection");

   norecovery = FALSE;     /* Flag we need a graceful shutdown after  */
                           /* Ctrl-BREAK                              */

   carrierDetect = FALSE;  /* No network connection yet             */

   connectionDied = FALSE; /* The connection hasn't failed yet */


   traceStart( name );

   portActive = TRUE;     /* record status for error handler */

   return FALSE;          /* Return success to caller                 */

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

   boolean firstPass = TRUE;

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
         static boolean recurse = FALSE;
         if ( ! recurse )
         {
            printmsg(2,"tsread: User aborted processing");
            recurse = TRUE;
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
            connectionDied = TRUE;
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

         firstPass = FALSE;

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
                 FALSE );

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
         connectionDied = TRUE;
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

   traceData( data, len, TRUE );

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

   portActive = FALSE;     /* flag port closed for error handler  */

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

   carrierDetect = FALSE;  /* No network connection yet               */
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
   hangupNeeded = FALSE;

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

   carrierDetect = FALSE;  /* No network connection yet               */

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

void tflowcontrol( boolean flow )
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

boolean tCD( void )
{
   boolean online = carrierDetect;

   return online;
} /* tCD */

/*--------------------------------------------------------------------*/
/*       b W a i t F o r N e t C o n n e c t                          */
/*                                                                    */
/*       Wait for remote system to connect to our waiting server      */
/*--------------------------------------------------------------------*/

boolean tWaitForNetConnect(const unsigned int timeout)
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
      return FALSE;
   }
   else if (nReady == 0)
   {
      printmsg(5, "WaitForNetConnect: select() timed out");
      return FALSE;
   }

   if (terminate_processing)
      return FALSE;

   connectedSock = accept(pollingSock, NULL, NULL);
   if (connectedSock == INVALID_SOCKET)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "WaitForNetConnect: could not accept a connection");
      printWSerror("accept", wsErr);
   }

   carrierDetect = TRUE;

   return TRUE;

} /* tWaitForNetConnect */
