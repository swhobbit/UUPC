/*--------------------------------------------------------------------*/
/*       u l i b i p . c                                              */
/*                                                                    */
/*       TCP/IP port communications driver for Windows sockets        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Copyright (c) David M. Watt 1993, All Right Reserved            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ulibip.c 1.3 1993/09/23 03:26:51 ahd Exp $
 *
 *    $Log: ulibip.c $
 * Revision 1.3  1993/09/23  03:26:51  ahd
 * Correct setting of carrier detect
 *
 * Revision 1.2  1993/09/21  01:42:13  ahd
 * Use standard MAXPACK limit for save buffer size
 *
 * Revision 1.1  1993/09/20  04:48:25  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <windows.h>
#include "winsock.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"
#include "ulib.h"
#include "comm.h"          // Modem status bits
#include "ssleep.h"
#include "catcher.h"

#include "commlib.h"       // Trace functions, etc.

#ifdef _Windows
#include "pwinsock.h"      // definitions for 16 bit Winsock functions
#endif

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

void AtWinsockExit(void);
boolean IsFatalSocketError(int err);
void tcloseline(void);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

static boolean carrierDetect = FALSE;

currentfile();
static boolean hangupNeeded = TRUE;
extern boolean winsockActive;                   // Initialized in catcher.c
static SOCKET pollingSock = INVALID_SOCKET;     // The current polling socket
static SOCKET connectedSock = INVALID_SOCKET;   // The currently connected socket
static boolean connectionDied = FALSE;          // The current connection failed

/*--------------------------------------------------------------------*/
/*                           Local defines                            */
/*--------------------------------------------------------------------*/

#ifndef MAKEWORD
#define MAKEWORD(a, b) ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#endif

/*--------------------------------------------------------------------*/
/*    I n i t W i n s o c k                                           */
/*                                                                    */
/*    Start the Windows sockets DLL                                   */
/*--------------------------------------------------------------------*/

boolean InitWinsock(void)
{
   WSADATA WSAData;
   int status;
   static boolean firstPass = TRUE;

   if ( winsockActive )
      return TRUE;

/*--------------------------------------------------------------------*/
/*       The atexit() must precede the WSAStartup() so the            */
/*       FreeLibrary() call gets done                                 */
/*--------------------------------------------------------------------*/

   if ( firstPass )
   {
      firstPass = FALSE;
      atexit(AtWinsockExit);
   }

#ifdef _Windows
   if (!pWinSockInit())
      return FALSE;
#endif

// status = WSAStartup(MAKEWORD(1,1), &WSAData);
   status = WSAStartup(0x0101, &WSAData);

   if (status != 0)
   {
      printf("WSAStartup Error: %d", status);
      return FALSE;
   }

   winsockActive = TRUE;
   return TRUE;

} /* InitWinsock */

/*--------------------------------------------------------------------*/
/*       A t W i n s o c k E x i t                                    */
/*                                                                    */
/*       Clean up Windows DLL at shutdown                             */
/*--------------------------------------------------------------------*/


void AtWinsockExit(void)
{
   WSACleanup();

#ifdef _Windows
   pWinSockExit();
#endif

   winsockActive = FALSE;

}  /* AtWinsockExit */

/*--------------------------------------------------------------------*/
/*    t o p e n a c t i v e                                           */
/*                                                                    */
/*    Open an active socket connection for I/O                        */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

int tactiveopenline(char *name, BPS bps, const boolean direct)
{
   SOCKADDR_IN sin;
   LPHOSTENT phe;
   LPSERVENT pse;

   if (!InitWinsock())           // Initialize library?
      return TRUE;               // No --> Report error

   if (portActive)              /* Was the port already active?      */
      closeline();               /* Yes --> Shutdown it before open  */

   printmsg(15, "tactiveopenline: %s", name);

   norecovery = FALSE;     // Flag we need a graceful shutdown after
                           // Ctrl-BREAK

   carrierDetect = FALSE;  /* No modem connected yet                */

   connectionDied = FALSE; /* The connection hasn't failed yet */

/*--------------------------------------------------------------------*/
/*                        Get remote host name                        */
/*--------------------------------------------------------------------*/

   sin.sin_family = AF_INET;
   phe = gethostbyname(name);

   if (phe)
#ifdef _Windows
      _fmemcpy((char FAR *) &(sin.sin_addr),
               (char FAR *) phe->h_addr,
               phe->h_length);
#else
      memcpy((char FAR *) &(sin.sin_addr),
             (char FAR *) phe->h_addr,
             phe->h_length);
#endif
   else {
      sin.sin_addr.s_addr = inet_addr(name);

      if ( sin.sin_addr.s_addr == INADDR_NONE )
      {
         printmsg(0, "tactiveopenline: gethostbyname returned %d.  "
            "Is '%s' listed in the hosts file or a valid IP address?",
            WSAGetLastError(), name);
         return TRUE;
      }

   } /* else */

/*--------------------------------------------------------------------*/
/*                     Get the TCP/IP port number                     */
/*--------------------------------------------------------------------*/

   pse = getservbyname("uucp", "tcp");
   if (pse == NULL)
   {
      sin.sin_port = 540;
      printmsg(0, "tactiveopenline: getservbyname returned %d, using port %d",
                  WSAGetLastError(),
                  (int) sin.sin_port);
   }
   else
      sin.sin_port = pse->s_port;

   connectedSock = socket( AF_INET, SOCK_STREAM, 0);
   if (connectedSock == INVALID_SOCKET)
   {
      printmsg(0, "tactiveopenline: socket() failed");
      return TRUE;
   }

   if (connect( connectedSock, (PSOCKADDR) &sin, sizeof(sin)) < 0)
   {
      printmsg(0, "tactiveopenline: connect() failed -- error %d",
         WSAGetLastError());
      closesocket( connectedSock );
      connectedSock = INVALID_SOCKET;

      return TRUE;
   }

   traceStart( name );

   portActive = TRUE;     /* record status for error handler */
   carrierDetect = TRUE;   // Carrier detect = connection

   return FALSE;                       // Return success to caller

} /* tactiveopenline */

/*--------------------------------------------------------------------*/
/*    t o p e n p a s s i v e                                         */
/*                                                                    */
/*    Listen on a socket for an incoming uucp connection              */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

int tpassiveopenline(char *name, BPS bps, const boolean direct)
{
   SOCKADDR_IN sin;
   LPSERVENT pse;

   if (!InitWinsock())           // Initialize library?
      return TRUE;               // No --> Report error

   if (portActive)              /* Was the port already active?      */
      closeline();               /* Yes --> Shutdown it before open  */

   printmsg(15, "tpassiveopenline: opening passive connection");

   norecovery = FALSE;     // Flag we need a graceful shutdown after
                           // Ctrl-BREAK

   carrierDetect = FALSE;  /* No network connection yet             */

   connectionDied = FALSE; /* The connection hasn't failed yet */

/*--------------------------------------------------------------------*/
/*                    Fill in host and family info                    */
/*--------------------------------------------------------------------*/

   sin.sin_family = AF_INET;

/*--------------------------------------------------------------------*/
/*                Fill in service information for tcp                 */
/*--------------------------------------------------------------------*/

   printmsg(15, "tpassiveopenline: doing getservbyname");
   pse = getservbyname("uucp", "tcp");

   if (pse == NULL)
   {
      sin.sin_port = 540;
      printmsg(0, "tpassiveopenline: getservbyname returned %d, using port %d",
                  WSAGetLastError(),
                  (int) sin.sin_port);
   }
   else
      sin.sin_port = pse->s_port;

   sin.sin_addr.s_addr = 0;

   printmsg(5, "tpassiveopenline: waiting on port %d",
               (int)ntohs(sin.sin_port));

/*--------------------------------------------------------------------*/
/*                     Create and bind TCP socket                     */
/*--------------------------------------------------------------------*/

   printmsg(15, "tpassiveopen: doing socket()");
   pollingSock = socket( AF_INET, SOCK_STREAM, 0);

   if (pollingSock == INVALID_SOCKET)
   {
      printmsg(0, "tpassiveopen: socket() failed, error = %d",
                  WSAGetLastError());
      return TRUE;
   }

   printmsg(15, "tpassiveopen: doing bind()");

   if (bind(pollingSock,
           (struct sockaddr FAR *) &sin,
           sizeof(sin)) == SOCKET_ERROR)
   {
      printmsg(0, "tpassiveopen: bind(pollingSock) failed: %d",
         WSAGetLastError());
      return TRUE;                      // report failure
   }

   printmsg(15, "tpassiveopen: doing listen()");
   if (listen(pollingSock, 2) == SOCKET_ERROR)
   {
      printmsg(0, "tpassiveopen: listen(pollingSock) failed: %d",
         WSAGetLastError());
      return TRUE;
   }

   traceStart( name );

   portActive = TRUE;     /* record status for error handler */

   return FALSE;          // Return success to caller

} /* tpassiveopen */

/*--------------------------------------------------------------------*/
/*    t s r e a d                                                     */
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

unsigned int tsread(char *output, unsigned int wanted, unsigned int timeout)
{
   fd_set readfds;
   struct timeval tm;
   int nReady;
   static char save[MAXPACK];
   static unsigned short bufsize = 0;
   time_t stop_time ;
   time_t now ;

/*--------------------------------------------------------------------*/
/*           Determine if our internal buffer has the data            */
/*--------------------------------------------------------------------*/

   if (bufsize >= wanted)
   {
      memmove( output, save, wanted );
      bufsize -= wanted;
      if ( bufsize )          /* Any data left over?                 */
         memmove( save, &save[wanted], bufsize );  /* Yes --> Save it*/
      return wanted + bufsize;
   } /* if */

   if (connectionDied || connectedSock == INVALID_SOCKET)
   {                             // Haven't accepted a connection yet?
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
      int needed = wanted - bufsize;

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
            printmsg(2,"sread: User aborted processing");
            recurse = TRUE;
         }
         return 0;
      }

/*--------------------------------------------------------------------*/
/*       Special case for network sockets: block for at least 5      */
/*       msec if we have to read at least one character (this         */
/*       needs to be tuned)                                           */
/*--------------------------------------------------------------------*/

      if (stop_time > now )
      {
         tm.tv_sec = stop_time - now;
         tm.tv_usec = 0;
      }
      else {

         tm.tv_usec = 5000;
         tm.tv_sec = 0;
      }

/*--------------------------------------------------------------------*/
/*                 Read the data from the socket                      */
/*--------------------------------------------------------------------*/

      nReady = select(1, &readfds, NULL, NULL, &tm);
      if (nReady == SOCKET_ERROR)
      {
         int err = WSAGetLastError();
         printmsg(0, "tsread: select error %d", err);
         if (IsFatalSocketError(err))
         {
            shutdown(connectedSock, 2);  // Fail both reads and writes
            connectionDied = TRUE;
         }
         bufsize = 0;
         return 0;
      }
      else if (nReady == 0)
      {
         printmsg(5, "tsread: timeout after %d seconds",timeout);
         bufsize = 0;
         return 0;
      }
      else {
         received = recv(connectedSock, &save[bufsize], needed, 0);
         if (received == SOCKET_ERROR)
         {
            printmsg(0, "tsread: recv() failed -- error %d",
               WSAGetLastError());
            bufsize = 0;
            return 0;
         }
      }  /* else */

#ifdef UDEBUG
      printmsg(15,"sread: Want %d characters, received %d, total %d in buffer",
            (int) wanted, (int) received, (int) bufsize + received);
#endif

/*--------------------------------------------------------------------*/
/*                    Log the newly received data                     */
/*--------------------------------------------------------------------*/

      traceData( &save[bufsize], received, FALSE );

/*--------------------------------------------------------------------*/
/*            If we got the data, return it to the caller             */
/*--------------------------------------------------------------------*/

      bufsize += received;
      if ( bufsize == wanted )
      {
         memmove( output, save, bufsize);
         bufsize = 0;

         if (debuglevel > 14)
            fwrite(output,1,bufsize,stdout);

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

   return bufsize;

} /* tsread */

/*--------------------------------------------------------------------*/
/*    t s w r i t e                                                   */
/*                                                                    */
/*    Write to the open socket                                        */
/*   Note:  this is non-blocking, so we've got to use select() to     */
/*    gradually write out the entire buffer                           */
/*--------------------------------------------------------------------*/

int tswrite(char *data, unsigned int len)
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
      printmsg(0, "tswrite: Error sending data to socket: %d",
         err);
      if (IsFatalSocketError(err))
      {
         shutdown(connectedSock, 2);  // Fail both reads and writes
         connectionDied = TRUE;
      }
      return 0;
   }

   if (status < len)
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
/*    t s s e n d b r k                                               */
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
/*    t c l o s e l i n e                                             */
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
/*    t h a n g u p                                                   */
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
/*    t S I O S p e e d                                               */
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
/*    t f l o w c o n t r o l                                         */
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
/*    t G e t S p e e d                                               */
/*                                                                    */
/*    Report current speed of communications connection               */
/*--------------------------------------------------------------------*/

BPS tGetSpeed( void )
{
   return 57600;           // Arbitary large number to avoid possible
                           // divide by zero error in caller
} /* GetSpeed */

/*--------------------------------------------------------------------*/
/*    t C D                                                           */
/*                                                                    */
/*    Report if we have carrier detect and lost it                    */
/*--------------------------------------------------------------------*/

boolean tCD( void )
{
   boolean online = carrierDetect;

   return online;
} /* tCD */

/*--------------------------------------------------------------------*/
/*       t W a i t F o r N e t C o n n e c t                          */
/*                                                                    */
/*       Wait for remote system to connect to our waiting server      */
/*--------------------------------------------------------------------*/

boolean tWaitForNetConnect(int timeout)
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
      printmsg(0, "WaitForNetConnect: select() failed, error %d",
         WSAGetLastError());
      return FALSE;
   }
   else if (nReady == 0)
   {
      printmsg(5, "WaitForNetConnect: select() timed out");
      return FALSE;
   }

   connectedSock = accept(pollingSock, NULL, NULL);
   if (connectedSock == INVALID_SOCKET)
   {
      printmsg(0, "WaitForNetConnect: could not connect -- error %d",
         WSAGetLastError());
   }

   carrierDetect = TRUE;

   return TRUE;

} /* tWaitForNetConnect */

boolean IsFatalSocketError(int err)
{
   if (err == WSAENOTSOCK     ||
       err == WSAENETDOWN     ||
       err == WSAENETRESET    ||
       err == WSAECONNABORTED ||
       err == WSAECONNRESET   ||
       err == WSAENOTCONN     ||
       err == WSAECONNREFUSED ||
       err == WSAEHOSTDOWN    ||
       err == WSAEHOSTUNREACH)
       return TRUE;
    else
       return FALSE;
}
