/*--------------------------------------------------------------------*/
/*       u l i b i p . c                                              */
/*                                                                    */
/*       TCP/IP port communications driver for Windows sockets        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Copyright (c) David M. Watt 1993, All Rights Reserved           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1995 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ulibip.c 1.24 1995/01/07 16:40:31 ahd Exp $
 *
 *    $Log: ulibip.c $
 *    Revision 1.24  1995/01/07 16:40:31  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.23  1994/12/22 00:37:05  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.22  1994/05/24 03:44:04  ahd
 *    Deleted unneeded declare for tcloseline()
 *
 *        Revision 1.21  1994/05/23  21:44:05  dmwatt
 *        Support h_nerr
 *
 *        Revision 1.19  1994/04/27  00:02:15  ahd
 *        Pick one: Hot handles support, OS/2 TCP/IP support,
 *                  title bar support
 *
 *        Revision 1.18  1994/03/28  02:18:45  ahd
 *        Cleanup header files
 *
 * Revision 1.17  1994/02/19  05:11:47  ahd
 * Use standard first header
 *
 * Revision 1.16  1994/01/01  19:21:42  ahd
 * Annual Copyright Update
 *
 * Revision 1.15  1993/12/30  02:56:28  dmwatt
 * Correct compile warnings
 *
 * Revision 1.14  1993/12/29  03:34:37  dmwatt
 * Corrected host to network short conversion
 *
 * Revision 1.13  1993/12/24  05:12:54  ahd
 * Use far buffer for master communications buffer
 *
 * Revision 1.12  1993/11/30  04:13:30  dmwatt
 * Optimize port processing
 *
 * Revision 1.12  1993/11/30  04:13:30  dmwatt
 * Optimize port processing
 *
 * Revision 1.11  1993/11/06  17:56:09  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.10  1993/10/30  22:07:49  dmwatt
 * Host byte ordering corrections
 *
 * Revision 1.9  1993/10/16  15:13:17  ahd
 * Allow parsing target port address when connecting to remote
 *
 * Revision 1.8  1993/10/12  01:33:23  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.7  1993/10/07  22:56:45  ahd
 * Use dynamically allocated buffer
 *
 * Revision 1.6  1993/10/02  23:12:35  dmwatt
 * Winsock error message support
 *
 * Revision 1.5  1993/09/26  03:32:27  dmwatt
 * Use Standard Windows NT error message module
 *
 * Revision 1.4  1993/09/25  03:07:56  ahd
 * Addition error traps by Dave Watt
 *
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

#include "uupcmoah.h"

#if defined(__OS2__)
#define OS2
#define BSD_SELECT
#include <types.h>
#include <sys\select.h>
#include <sys\socket.h>
#include <netinet\in.h>
#include <netdb.h>
#include <utils.h>
#include <sys\time.h>
#include <nerrno.h>

#define WSAGetLastError() sock_errno()
#define closesocket(s) soclose(s)
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SOCKADDR_IN struct sockaddr_in
#define LPHOSTENT struct hostent *
#define LPSERVENT struct servent *
#define PSOCKADDR struct sockaddr *

#include "psos2err.h"        /* Windows sockets error messages        */
#include "catcher.h"         /* For norecovery declaration            */

#else /* DOS and Windows */
#include <windows.h>
#include "winsock.h"

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "ulibip.h"
#include "catcher.h"

#include "pwserr.h"        /* Windows sockets error messages           */

#ifdef WIN32
#include "pnterr.h"
#endif

#ifdef _Windows
#include "pwinsock.h"      /* definitions for 16 bit Winsock functions  */
#endif

#endif
#include "commlib.h"       /* Trace functions, etc.                    */

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

#ifdef __OS2__
typedef int SOCKET;
#endif

#if !defined(__OS2__)
void AtWinsockExit(void);
#endif
KWBoolean IsFatalSocketError(int err);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();
static KWBoolean hangupNeeded = KWTrue;

#if defined(__OS2__)
KWBoolean winsockActive = KWFalse;  /* Initialized here -- <not> in catcher.c
                                     No need for catcher -- no WSACleanup() of
                                     OS/2 sockets required                  */
#else
extern KWBoolean winsockActive;                  /* Initialized in catcher.c  */
#endif

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
/*    I n i t W i n s o c k                                           */
/*                                                                    */
/*    Start the Windows sockets DLL                                   */
/*--------------------------------------------------------------------*/

KWBoolean InitWinsock(void)
{
#if !defined(__OS2__)
   WSADATA WSAData;
#endif
   int status;
   static KWBoolean firstPass = KWTrue;

   if ( winsockActive )
      return KWTrue;

/*--------------------------------------------------------------------*/
/*       The atexit() must precede the WSAStartup() so the            */
/*       FreeLibrary() call gets done                                 */
/*--------------------------------------------------------------------*/

#if !defined(__OS2__)
   if ( firstPass )
   {
      firstPass = KWFalse;
      atexit(AtWinsockExit);
   }
#endif

#ifdef _Windows
   if (!pWinSockInit())
      return KWFalse;
#endif

/* status = WSAStartup(MAKEWORD(1,1), &WSAData); */

#if defined(__OS2__)
   status = sock_init();
#else
   status = WSAStartup(0x0101, &WSAData);
#endif

   if (status != 0)
   {
#if defined(__OS2__)
      printf("sock_init Error: %d", status);
#else
      printf("WSAStartup Error: %d", status);
#endif
      return KWFalse;
   }

   winsockActive = KWTrue;
   return KWTrue;

} /* InitWinsock */

#if !defined(__OS2__)
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

   winsockActive = KWFalse;

}  /* AtWinsockExit */

#endif

/*--------------------------------------------------------------------*/
/*    t o p e n a c t i v e                                           */
/*                                                                    */
/*    Open an active socket connection for I/O                        */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

int tactiveopenline(char *name, BPS bps, const KWBoolean direct)
{
   SOCKADDR_IN sin;
   LPHOSTENT phe;
   LPSERVENT pse;
   u_short remotePort;
   char *portStr;

   if (!InitWinsock())           /* Initialize library?               */
      return KWTrue;              /* No --> Report error               */

   if (portActive)              /* Was the port already active?      */
      closeline();               /* Yes --> Shutdown it before open  */

   printmsg(15, "tactiveopenline: %s", name);

   norecovery = KWFalse;    /* Flag we need a graceful shutdown after  */
                           /* Ctrl-BREAK                              */

   connectionDied = KWFalse; /* The connection hasn't failed yet */

/*--------------------------------------------------------------------*/
/*                        Parse out port address                      */
/*--------------------------------------------------------------------*/
   remotePort = 0;
   portStr = strchr(name, ':');
   if (portStr)
   {
         *portStr = '\0';
         portStr++;

         pse = getservbyname(portStr, "tcp");
         if (pse == NULL)
         {
            remotePort = (u_short)atoi(portStr);
            printmsg(4, "tactiveopenline: connecting to remote port %d",
               (int)remotePort);

            /* Remember to invert byte order! */

            remotePort = htons(remotePort);
         } else {
            remotePort = pse->s_port;
         }

   }

/*--------------------------------------------------------------------*/
/*                        Get remote host name                        */
/*--------------------------------------------------------------------*/

   sin.sin_family = AF_INET;
   phe = gethostbyname(name);

   if (phe)
      MEMCPY(&(sin.sin_addr), phe->h_addr, phe->h_length);
   else {
      sin.sin_addr.s_addr = inet_addr(name);

      if ( sin.sin_addr.s_addr == INADDR_NONE )
      {
         int wsErr = h_errno;

         printmsg(0, "tactiveopenline: "
            "Is '%s' listed in the hosts file or a valid IP address?",
            name);
         printWSerror("gethostbyname", wsErr);
         return KWTrue;
      }

   } /* else */

/*--------------------------------------------------------------------*/
/*                     Get the TCP/IP port number                     */
/*--------------------------------------------------------------------*/

   if (remotePort == 0)
   {
      pse = getservbyname("uucp", "tcp");
      if (pse == NULL)
      {
         int wsErr = WSAGetLastError();

         sin.sin_port = htons(540);
         printWSerror("getservbyname", wsErr);
         printmsg(0, "tactiveopenline: using port %d", (int)htons(sin.sin_port));
      }
      else
         sin.sin_port = pse->s_port;
   }
   else /* Remember to invert byte order! */
      sin.sin_port = remotePort;

   connectedSock = socket( AF_INET, SOCK_STREAM, 0);
   if (connectedSock == INVALID_SOCKET)
   {
      printmsg(0, "tactiveopenline: socket() failed");
      return KWTrue;
   }

   if (connect( connectedSock, (PSOCKADDR) &sin, sizeof(sin)) < 0)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "tactiveopenline: connect() failed");
      printWSerror("connect", wsErr);
      closesocket( connectedSock );
      connectedSock = INVALID_SOCKET;

      return KWTrue;
   }

   traceStart( name );

   portActive = KWTrue;    /* record status for error handler */

   return KWFalse;                      /* Return success to caller     */

} /* tactiveopenline */

/*--------------------------------------------------------------------*/
/*    t o p e n p a s s i v e                                         */
/*                                                                    */
/*    Listen on a socket for an incoming uucp connection              */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

int tpassiveopenline(char *name, BPS bps, const KWBoolean direct)
{
   SOCKADDR_IN sin;
   LPSERVENT pse;

   if (!InitWinsock())           /* Initialize library?               */
      return KWTrue;              /* No --> Report error               */

   if (portActive)              /* Was the port already active?      */
      closeline();               /* Yes --> Shutdown it before open  */

   printmsg(15, "tpassiveopenline: opening passive connection");

   norecovery = KWFalse;    /* Flag we need a graceful shutdown after */
                           /* Ctrl-BREAK                             */
   connectionDied = KWFalse; /* The connection hasn't failed yet      */

/*--------------------------------------------------------------------*/
/*      Handle being started from INET, we already have a socket      */
/*--------------------------------------------------------------------*/

   if ( connectedSock != INVALID_SOCKET )
   {
      traceStart( name );
      portActive = KWTrue;    /* record status for error handler      */
      return KWFalse;         /* Return success to caller             */
   }

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
      int wsErr = WSAGetLastError();

      sin.sin_port = htons(540);
      printWSerror("getservbyname", wsErr);
      printmsg(0, "tpassiveopenline: using port %d",
                  (int)ntohs(sin.sin_port));
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
      int wsErr = WSAGetLastError();

      printmsg(0, "tpassiveopen: socket() failed");
      printWSerror("socket", wsErr);
      return KWTrue;
   }

   printmsg(15, "tpassiveopen: doing bind()");

   if (bind(pollingSock,
           (struct sockaddr UUFAR *) &sin,
           sizeof(sin)) == SOCKET_ERROR)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "tpassiveopen: bind(pollingSock) failed");
      printWSerror("bind", wsErr);
      return KWTrue;                     /* report failure              */
   }

   printmsg(15, "tpassiveopen: doing listen()");
   if (listen(pollingSock, 2) == SOCKET_ERROR)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "tpassiveopen: listen(pollingSock) failed");
      printWSerror("listen", wsErr);
      return KWTrue;
   }

   traceStart( name );

   portActive = KWTrue;    /* record status for error handler */

   return KWFalse;         /* Return success to caller                 */

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
/*       Special case for network sockets: block for at least 5       */
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

      nReady = select(connectedSock + 1, &readfds, NULL, NULL, &tm);

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

         if ( received == 0 )
         {
            printmsg(0, "tsread: EOF on recv()");
            commBufferUsed = 0;
            return 0;

         }
         else if (received == SOCKET_ERROR)
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
/*    t s w r i t e                                                   */
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

   status = send(connectedSock, (char UUFAR *)data, len, 0);

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

   hangupNeeded = KWFalse;
   connectionDied = KWFalse;

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

void tflowcontrol( KWBoolean flow )
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
   return 57600;           /* Arbitary large number to avoid possible  */
                           /* divide by zero error in caller           */
} /* GetSpeed */

/*--------------------------------------------------------------------*/
/*    t C D                                                           */
/*                                                                    */
/*    Report if we have carrier detect and lost it                    */
/*--------------------------------------------------------------------*/

KWBoolean tCD( void )
{

   return connectionDied ? KWFalse : KWTrue;

} /* tCD */

/*--------------------------------------------------------------------*/
/*       t W a i t F o r N e t C o n n e c t                          */
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

   nReady = select(pollingSock + 1, &readfds, NULL, NULL, &tm);

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

   return KWTrue;

} /* tWaitForNetConnect */

/*--------------------------------------------------------------------*/
/*      I s F a t a l S o c k e t E r r o r                           */
/*                                                                    */
/*      Determine if an error is a show stopped                       */
/*--------------------------------------------------------------------*/

KWBoolean IsFatalSocketError(int err)
{
#if defined(__OS2__)
   if (err == ENOTSOCK     ||
       err == ENETDOWN     ||
       err == ENETRESET    ||
       err == ECONNABORTED ||
       err == ECONNRESET   ||
       err == ENOTCONN     ||
       err == ECONNREFUSED ||
       err == EHOSTDOWN    ||
       err == EHOSTUNREACH)
#else
   if (err == WSAENOTSOCK     ||
       err == WSAENETDOWN     ||
       err == WSAENETRESET    ||
       err == WSAECONNABORTED ||
       err == WSAECONNRESET   ||
       err == WSAENOTCONN     ||
       err == WSAECONNREFUSED ||
       err == WSAEHOSTDOWN    ||
       err == WSAEHOSTUNREACH)
#endif
       return KWTrue;
    else
       return KWFalse;

} /* IsFatalSocketError */

#ifdef WIN32
BOOL AbortNetwork(void)
{
   if (connectedSock != INVALID_SOCKET) {
      closesocket(connectedSock);
      connectedSock = INVALID_SOCKET;
   }

   if (pollingSock != INVALID_SOCKET) {
      closesocket(pollingSock);
      pollingSock = INVALID_SOCKET;
   }

   return KWFalse;
}
#endif

/*--------------------------------------------------------------------*/
/*       t G e t C o m H a n d l e                                    */
/*                                                                    */
/*       Return current socket number to caller                       */
/*--------------------------------------------------------------------*/

int tGetComHandle( void )
{
   return (int) connectedSock;
}

/*--------------------------------------------------------------------*/
/*       t S e t C o m H a n d l e                                    */
/*                                                                    */
/*       Set socket number for hot login (start by INETD)             */
/*--------------------------------------------------------------------*/

void tSetComHandle( const int sock )
{
   connectedSock = sock;
}
