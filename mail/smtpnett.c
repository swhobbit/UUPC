/*--------------------------------------------------------------------*/
/*       s m t p n e t t . c                                          */
/*                                                                    */
/*       TCP/IP generic support for UUPC/extended SMTP                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2002 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                            Header files                            */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include "smtpnett.h"
#include "catcher.h"
#include "ssleep.h"

#include <limits.h>

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpnett.c 1.4 2001/03/12 13:56:08 ahd v1-13k $
 *
 *    $Log: smtpnett.c $
 *    Revision 1.4  2001/03/12 13:56:08  ahd
 *    Annual Copyright update
 *
 *    Revision 1.3  2000/05/25 03:41:49  ahd
 *    Use more conservative buffering to avoid aborts
 *
 *    Revision 1.2  2000/05/12 12:36:30  ahd
 *    Annual copyright update
 *
 *    Revision 1.1  1999/02/21 04:09:32  ahd
 *    Initial revision
 *
 */


#if defined(__OS2__)
KWBoolean winsockActive = KWFalse;  /* Initialized here -- <not> in catcher.c
                                     No need for catcher -- no WSACleanup() of
                                     OS/2 sockets required                  */
#else
extern KWBoolean winsockActive;                 /* Initialized in catcher.c  */
#endif

#if !defined(__OS2__)
void AtWinsockExit(void);
#endif

/*--------------------------------------------------------------------*/
/*                          Local prototypes                          */
/*--------------------------------------------------------------------*/

#define MINUTE(seconds) ((seconds)*60)

/*--------------------------------------------------------------------*/
/*                      Global defines/variables                      */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtpnett.c 1.4 2001/03/12 13:56:08 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*       g e t M o d e T i m e o u t                                  */
/*                                                                    */
/*       Determine timeout for specified client mode                  */
/*--------------------------------------------------------------------*/

time_t
getModeTimeout(SMTPMode mode)
{

   switch(mode)
   {
      case SM_MASTER:             return LONG_MAX;
      case SM_UNGREETED:          return MINUTE(5);
      case SM_IDLE:               return MINUTE(10);
      case SM_ADDR_FIRST:         return MINUTE(5);
      case SM_ADDR_SECOND:        return MINUTE(5);
      case SM_DATA:               return MINUTE(15);

      default:                    return MINUTE(1);

   } /* switch(mode) */

} /* getModeTimeout */

/*--------------------------------------------------------------------*/
/*    InitializeNetwork                                               */
/*                                                                    */
/*    Start the Windows sockets DLL                                   */
/*--------------------------------------------------------------------*/

KWBoolean
InitializeNetwork (void)
{

#if !defined(__OS2__)
   static KWBoolean firstPass = KWTrue;
   WSADATA WSAData;
#endif

   int status;

   if (winsockActive)
      return KWTrue;

/*--------------------------------------------------------------------*/
/*       The atexit() must precede the WSAStartup() so the            */
/*       FreeLibrary() call gets done                                 */
/*--------------------------------------------------------------------*/

#if !defined(__OS2__)
   if (firstPass)
   {
      firstPass = KWFalse;
      atexit(AtWinsockExit);
   }
#endif

#ifdef _Windows
   if (!pWinSockInit())
      return KWFalse;
#endif

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

} /* InitializeNetwork */

#if !defined(__OS2__)
/*--------------------------------------------------------------------*/
/*       A t W i n s o c k E x i t                                    */
/*                                                                    */
/*       Clean up Windows DLL at shutdown                             */
/*--------------------------------------------------------------------*/

void
AtWinsockExit(void)
{
   WSACleanup();

#ifdef _Windows
   pWinSockExit();
#endif

   winsockActive = KWFalse;

}  /* AtWinsockExit */

#endif

/*--------------------------------------------------------------------*/
/*       o p e n M a s t e r                                          */
/*                                                                    */
/*       Listen on a socket for an incoming uucp connection; this     */
/*       a stripped down version of the original passive openline     */
/*       in ulibip.c.                                                 */
/*--------------------------------------------------------------------*/

SOCKET
openMaster(const char *name)
{
   static const char mName[] = "masterOpen";

   SOCKET pollingSock;
   SOCKADDR_IN sin;
   LPSERVENT pse;
   int sockopt = 1;

   norecovery = KWFalse;            /* Flag we need a graceful
                                       shutdown after Ctrl-BREAK      */

/*--------------------------------------------------------------------*/
/*                Fill in service information for tcp                 */
/*--------------------------------------------------------------------*/

   printmsg(NETDEBUG, "%s: determining port for %s",
                     mName,
                     name);

   if (isdigit(*name))
      sin.sin_port = htons((u_short) atoi(name));
   else if ((pse = getservbyname((char *) name, "tcp")) == NULL)
   {
      int wsErr = WSAGetLastError();
      printWSerror("getservbyname", wsErr);
      return INVALID_SOCKET;
   }
   else
      sin.sin_port = pse->s_port;

   sin.sin_family = AF_INET;     /* A internet socket, of course     */
   sin.sin_addr.s_addr = 0;      /* Listen on all interfaces         */

/*--------------------------------------------------------------------*/
/*                     Create and bind TCP socket                     */
/*--------------------------------------------------------------------*/

   printmsg(NETDEBUG + 1, "%s: doing socket()", mName);

   pollingSock = socket(AF_INET, SOCK_STREAM, 0);

   if (pollingSock == INVALID_SOCKET)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "%s: socket() failed", mName);
      printWSerror("socket", wsErr);
      return INVALID_SOCKET;
   }

   printmsg(NETDEBUG, "%s: doing bind() on socket %d port %d",
                      mName,
                      pollingSock,
                      (int) ntohs(sin.sin_port));

   if (bind(pollingSock,
           (struct sockaddr UUFAR *) (void *) &sin,
           sizeof(sin)) == SOCKET_ERROR)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "%s: bind(pollingSock) failed", mName);
      printWSerror("bind", wsErr);
      closeSocket(pollingSock);
      return INVALID_SOCKET;        /* report failure            */
   }

   printmsg(NETDEBUG + 1, "%s: doing setsockopt()", mName);

   if (setsockopt(pollingSock, SOL_SOCKET, SO_REUSEADDR,
         (char UUFAR *)&sockopt, sizeof sockopt) == SOCKET_ERROR)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "%s: setsockopt() failed", mName);
      printWSerror("setsockopt", wsErr);
      closeSocket(pollingSock);
      return INVALID_SOCKET;
   }

   printmsg(NETDEBUG, "%s: doing listen()", mName);

   if (listen(pollingSock, SOMAXCONN) == SOCKET_ERROR)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "%s: listen(pollingSock) failed", mName);
      printWSerror("listen", wsErr);
      closeSocket(pollingSock);
      return INVALID_SOCKET;
   }

   return pollingSock;              /* Return success to caller      */

} /* openMaster */

/*--------------------------------------------------------------------*/
/*       o p e n S l a v e                                            */
/*                                                                    */
/*       Given a listening socket ready with a new connection,        */
/*       accept the new connection and return the resulting socket    */
/*--------------------------------------------------------------------*/

SOCKET
openSlave(SOCKET pollingSock)
{
   static const char mName[] = "openSlave";
   int bufferSize = 31 * 1024;

   SOCKET connectedSock = accept(pollingSock, NULL, NULL);

   if (connectedSock == INVALID_SOCKET)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "%s: could not accept a connection", mName);
      printWSerror("accept", wsErr);
   }

#ifdef UDEBUG
   if (debuglevel > 6)
   {
      static int option[] = { SO_SNDBUF, SO_RCVBUF };
      static int optionCount = (sizeof option / sizeof option[0]);
      int subscript;

      for ( subscript = 0;
            subscript < optionCount;
            subscript ++ )
      {
         int sockopt = 0xdeadbeef;
         int optlen = sizeof sockopt;

         if ( getsockopt( connectedSock,
                          SOL_SOCKET,
                          option[subscript],
                          (char UUFAR *) &sockopt,
                          &optlen ))
         {
            printmsg(0,"%s: getsockopt(%d, SOL_SOCKET, %0d, &%08x, &%d) failed",
                      mName,
                      connectedSock,
                      option[subscript],
                      sockopt,
                      sizeof sockopt );
         }
         else {
            printmsg(NETDEBUG, "%s: Socket %d option %d is x%08x",
                     mName,
                     connectedSock,
                     option[subscript],
                     sockopt );
         } /* else */

      } /* for */

   } /* if ( debuglevel > NETDEBUG } */

#endif

/*--------------------------------------------------------------------*/
/*                    Set transmission buffer size                    */
/*--------------------------------------------------------------------*/

   printmsg(NETDEBUG + 1,"%s: setsockopt(%d, SOL_SOCKET, SO_SNDBUF, &%08x, %d)",
             mName,
             connectedSock,
             bufferSize,
             sizeof bufferSize );

   if (setsockopt(connectedSock,
                  SOL_SOCKET,
                  SO_SNDBUF,
                  (char UUFAR *) &bufferSize,
                  sizeof bufferSize) == SOCKET_ERROR)
   {
      int wsErr = WSAGetLastError();

      printmsg(0,"%s: setsockopt(%d, SOL_SOCKET, SO_SNDBUF, &%08x, %d) failed",
                mName,
                connectedSock,
                bufferSize,
                sizeof bufferSize );
      printWSerror("setsockopt", wsErr);
   }

/*--------------------------------------------------------------------*/
/*                    Set receive buffer size                         */
/*--------------------------------------------------------------------*/

   printmsg(NETDEBUG + 1,"%s: setsockopt(%d, SOL_SOCKET, SO_RCVBUF, &%08x, %d)",
             mName,
             connectedSock,
             bufferSize,
             sizeof bufferSize );

   if (setsockopt(connectedSock, SOL_SOCKET, SO_RCVBUF,
         (char UUFAR *)&bufferSize, sizeof bufferSize) == SOCKET_ERROR)
   {
      int wsErr = WSAGetLastError();

      printmsg(0,"%s: setsockopt(%d, SOL_SOCKET, SO_RCVBUF, &%08x) failed",
                mName,
                connectedSock,
                bufferSize );
      printWSerror("setsockopt", wsErr);
   }

   return connectedSock;

} /* openSlave */

/*--------------------------------------------------------------------*/
/*       S M T P W r i t e                                            */
/*                                                                    */
/*       Write to the open socket                                     */
/*--------------------------------------------------------------------*/

size_t
SMTPWrite(SMTPClient *client,
          const char UUFAR *data,
          unsigned int len)
{
   static const char mName[] = "SMTPWrite";
   int status;
   static const size_t maxWrite = 1024;

   assertSMTP(client);

   status = send(getClientHandle(client),
                 (char UUFAR *)data,
                 (int) len,
                 0);

   if (status == SOCKET_ERROR)
   {
      int wsErr;

      wsErr = WSAGetLastError();
      printmsg(0, "%s: Error writing %u bytes to socket %d",
                  mName,
                  len,
                  getClientHandle(client));
      printWSerror("send", wsErr);

      /* Flag the error to client and return error */
      setClientSocketError(client, wsErr);
      return 0;
   }

   if (status < (int)len)     /* Breaks if len > 32K, which is unlikely */
      printmsg(0,"%s: Write to network failed, "
                  "wanted to write %u and only wrote %d.",
                  mName,
                  len,
                  status);

/*--------------------------------------------------------------------*/
/*              Return byte count transmitted to caller               */
/*--------------------------------------------------------------------*/

   return status;

} /* SMTPWrite */

/*--------------------------------------------------------------------*/
/*       S M T P R e a d                                              */
/*                                                                    */
/*       Perform a read off the network to (perhaps) fill in the      */
/*       supplied client read buffer; should be called only after     */
/*       select determines client is ready.                           */
/*--------------------------------------------------------------------*/

size_t
SMTPRead(SMTPClient *client)
{
   static const char mName[] = "SMTPRead";
   int received;

   assertSMTP(client);

/*--------------------------------------------------------------------*/
/*               Reset flag which drives our invocation               */
/*--------------------------------------------------------------------*/

   setClientReady(client, KWFalse);

/*--------------------------------------------------------------------*/
/*                If no more data from client, return                 */
/*--------------------------------------------------------------------*/

   if (client->endOfTransmission)
      return client->receive.NetworkUsed;

/*--------------------------------------------------------------------*/
/*                 Make sure our buffer is big enough                 */
/*--------------------------------------------------------------------*/

   if (client->receive.NetworkUsed > client->receive.NetworkAllocated)
   {
         printmsg(2, "%s: Client %d buffer overflowed -- "
                            " %d bytes allocated, %d bytes used",
                    mName,
                    getClientSequence(client),
                    client->receive.NetworkAllocated,
                    client->receive.NetworkUsed);
        panic();
   }
   else if (client->receive.NetworkUsed == client->receive.NetworkAllocated)
   {
      if (client->receive.NetworkAllocated < MAX_BUFFER_SIZE)
      {
         client->receive.NetworkAllocated *= 2;
         printmsg(2, "%s: Client %d buffer size doubled to %d bytes",
                    mName,
                    getClientSequence(client),
                    client->receive.NetworkAllocated);
         client->receive.NetworkBuffer =
                       realloc(client->receive.NetworkBuffer,
                                 client->receive.NetworkAllocated);
         checkref(client->receive.NetworkBuffer);

      } /* if (client->receive.NetworkAllocated < MAX_BUFFER_SIZE) */
      else {
          printmsg(0, "%s: Client %d used all %d bytes of network buffer,"
                      " truncated.",
                      mName,
                      getClientSequence(client),
                      client->receive.NetworkAllocated);
          return client->receive.NetworkUsed;

      } /* else */

   } /* if (client->receive.NetworkUsed == client->receive.NetworkAllocated) */

/*--------------------------------------------------------------------*/
/*                  Actually get our next data read                   */
/*--------------------------------------------------------------------*/

   received = recv(getClientHandle(client),
                   client->receive.NetworkBuffer + client->receive.NetworkUsed,
                   (int) (client->receive.NetworkAllocated - client->receive.NetworkUsed),
                   0);

   if (received == 0)
   {
      client->endOfTransmission = KWTrue;
      printmsg(0, "%s: client %d EOF on recv(%ld,%p,%d,%d)",
                  mName,
                  getClientSequence(client),
                  (long) getClientHandle(client),
                  client->receive.NetworkBuffer + client->receive.NetworkUsed,
                  (int) (client->receive.NetworkAllocated - client->receive.NetworkUsed),
                  0);
   }
   else if (received == SOCKET_ERROR)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "%s: client %d recv() failed",
                  mName,
                  getClientSequence(client));
      printWSerror("recv", wsErr);

      /* Flag the error to client and return error */
      setClientSocketError(client, wsErr);
      return 0;

   }
   else {
      incrementClientBytesRead(client, (size_t) received);
      client->receive.NetworkUsed += (size_t) received;
   }

   return client->receive.NetworkUsed;

} /* SMTPRead */

/*--------------------------------------------------------------------*/
/*       c l o s e S o c k e t                                        */
/*                                                                    */
/*       Close a socket, terminating all socket processing if this    */
/*       is the last one.  Must be called for ALL sockets for which   */
/*       socketsOpen is incremented                                   */
/*--------------------------------------------------------------------*/

void
closeSocket(SOCKET handle)
{
   static const char mName[] = "closeSocket";

   if (handle == INVALID_SOCKET)
   {
      printmsg(0, "%s: Called for invalid socket", mName);
      panic();
   }

   closesocket(handle);

} /* closeSocket */

/*--------------------------------------------------------------------*/
/*       s e l e c t R e a d y S o c k e t s                          */
/*                                                                    */
/*       Perform select to determine what sockets are ready,          */
/*       waiting if needed                                            */
/*--------------------------------------------------------------------*/

KWBoolean
selectReadySockets(SMTPClient *master)
{
   static const char mName[] = "flagReadySockets";
   SMTPClient *current = master;

   fd_set readfds;
   int nReady;
   int nSelected = 0;
   int nTotal = 0;
   int maxSocket = 0;
   struct timeval timeoutPeriod;

   timeoutPeriod.tv_sec = 30;
   timeoutPeriod.tv_usec = 0;

   FD_ZERO(&readfds);

/*--------------------------------------------------------------------*/
/*       Loop through the list of valid sockets, adding each one      */
/*       to the list of sockets to check and determining the          */
/*       shortest timeout of the sockets.                             */
/*--------------------------------------------------------------------*/

   do {

      assertSMTP(current);

      if (isClientValid(current) &&
           ! isClientIgnored(current) &&
           ! getClientReady(current))
      {

            FD_SET(((unsigned)getClientHandle(current)), &readfds);

            if ((int) getClientHandle(current) >= maxSocket)
               maxSocket = getClientHandle(current) + 1;

            nSelected++;
      }

      /* If we can better the timeout period, examine current client */
      if (timeoutPeriod.tv_sec > 0)
      {
         unsigned long timeout = (unsigned long) getClientTimeout(current);
         if (timeout < (unsigned long) timeoutPeriod.tv_sec)
             timeoutPeriod.tv_sec = timeout;
      }

      nTotal++;
      current = current->next;

   } while(current);

/*--------------------------------------------------------------------*/
/*       If no sockets are to be checked and no clients are           */
/*       already in a state to processed, report the oddity           */
/*       and continue after a short pause                             */
/*--------------------------------------------------------------------*/

   if (! nSelected)
   {
      if (timeoutPeriod.tv_sec > 0)
      {
         printmsg(0, "%s: All sockets of %d ignored!",
                     mName,
                     nTotal);
         ssleep(1);
      }
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*             Perform actual selection and check for errors          */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   printmsg(5, "%s: Selecting total of %d sockets (through %d) for timeout of %d seconds",
               mName,
               nSelected,
               maxSocket,
               (long) timeoutPeriod.tv_sec);
#endif

   nReady = select(maxSocket, &readfds, NULL, NULL, &timeoutPeriod);

   if (nReady == SOCKET_ERROR)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "%s: select() of %d (out of %d) sockets failed" ,
               mName,
               nSelected,
               nTotal);
      printWSerror("select", wsErr);
      panic();
   }

#ifdef UDEBUG
   printmsg(5, "%s: %d of %d (out of %d) sockets were ready.",
             mName,
             nReady,
             nSelected,
             nTotal);
#endif

/*--------------------------------------------------------------------*/
/*                   Update list of sockets to process                */
/*--------------------------------------------------------------------*/

   current = master;

   do {
      if (isClientValid(current) &&
          FD_ISSET(((unsigned) getClientHandle(current)), &readfds))
      {
         setClientReady(current, KWTrue);
         setClientProcess(current, KWTrue);
      }

      current = current->next;

   } while(current);

   return KWTrue;

} /* flagReadySockets */

/*--------------------------------------------------------------------*/
/*       i s S o c k e t R e a d y                                    */
/*                                                                    */
/*       Perform select to determine what sockets are ready,          */
/*       waiting if needed                                            */
/*--------------------------------------------------------------------*/

KWBoolean
isSocketReady(SMTPClient *current, time_t timeout)
{
   static const char mName[] = "isSocketReady";

   fd_set readfds;
   int nReady;
   int nSelected = 0;
   int nTotal = 0;
   int maxSocket = 0;
   struct timeval timeoutPeriod;

   timeoutPeriod.tv_sec = timeout;
   timeoutPeriod.tv_usec = 1;

   FD_ZERO(&readfds);

/*--------------------------------------------------------------------*/
/*       Loop through the list of valid sockets, adding each one      */
/*       to the list of sockets to check and determining the          */
/*       shortest timeout of the sockets.                             */
/*--------------------------------------------------------------------*/

   assertSMTP(current);

   if (!isClientValid(current) || isClientIgnored(current))
      return KWFalse;

   if (getClientReady(current))
      return KWTrue;

   FD_SET(((unsigned)getClientHandle(current)), &readfds);
   maxSocket = getClientHandle(current) + 1;

/*--------------------------------------------------------------------*/
/*             Perform actual selection and check for errors          */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   printmsg(5, "%s: Selecting total of %d sockets (through %d) for timeout of %d seconds",
               mName,
               1,
               maxSocket,
               (long) timeoutPeriod.tv_sec);
#endif

   nReady = select(maxSocket, &readfds, NULL, NULL, &timeoutPeriod);

   if (nReady == SOCKET_ERROR)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "%s: select() of %d (out of %d) sockets failed" ,
               mName,
               nSelected,
               nTotal);
      printWSerror("select", wsErr);
      panic();
   }

/*--------------------------------------------------------------------*/
/*                 Determine if our socket was ready                  */
/*--------------------------------------------------------------------*/

   if (FD_ISSET(((unsigned) getClientHandle(current)), &readfds))
   {
      printmsg(4,"%s: Client %d (socket %d) is ready",
                  mName,
                  getClientSequence(current),
                  getClientHandle(current));

      setClientReady(current, KWTrue);
      setClientProcess(current, KWTrue);
      return KWTrue;
   }

   return KWFalse;

} /* isSocketReady */

/*--------------------------------------------------------------------*/
/*       g e t D e f a u l t H a n d l e                              */
/*                                                                    */
/*       Report default handle to initialize program to               */
/*--------------------------------------------------------------------*/

SOCKET
getDefaultHandle( void )
{
   return INVALID_SOCKET;
} /* getDefaultHandle */
