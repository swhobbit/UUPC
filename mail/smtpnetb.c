
/*--------------------------------------------------------------------*/
/*       s m t p n e t w . c                                          */
/*                                                                    */
/*       SMTP verb parser for UUPC/extended                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1999 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpnetw.c 1.24 1999/01/08 02:21:05 ahd Exp $
 *
 *    $Log: smtpnetw.c $
 *    Revision 1.24  1999/01/08 02:21:05  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.23  1999/01/04 03:54:27  ahd
 *    Annual copyright change
 *
 *    Revision 1.22  1998/11/04 01:59:55  ahd
 *    Prevent buffer overflows when processing UIDL lines
 *    in POP3 mail.
 *    Add internal sanity checks for various client structures
 *    Convert various files to CR/LF from LF terminated lines
 *
 * Revision 1.21  1998/05/11  13:55:28  ahd
 * Reduce buffer size to handle OS/2 builds
 *
 *    Revision 1.20  1998/05/11 01:20:48  ahd
 *    Length line of trace messages in log
 *
 *    Revision 1.19  1998/04/27 01:45:15  ahd
 *    Revamp bufferring for larger buffers
 *
 *    Revision 1.18  1998/04/24 03:30:13  ahd
 *    Use local buffers, not client->transmit.buffer, for output
 *    Rename receive buffer, use pointer into buffer rather than
 *         moving buffered data to front of buffer every line
 *    Restructure main processing loop to give more priority
 *         to client processing data already buffered
 *    Add flag bits to client structure
 *    Add flag bits to verb tables
 *
 *    Revision 1.17  1998/04/22 01:19:54  ahd
 *    Performance improvements for SMTPD data mode
 *
 *    Revision 1.16  1998/04/19 15:30:08  ahd
 *    Improved error messages for network errors
 *
 *    Revision 1.15  1998/04/08 11:35:35  ahd
 *    CHange error processing for bad sockets
 *
 *    Revision 1.14  1998/03/16 07:49:07  ahd
 *    Make NETSCAPE send CR/LF
 *
 *    Revision 1.13  1998/03/16 06:42:49  ahd
 *    Allow larger receive buffers
 *
 *    Revision 1.12  1998/03/08 23:10:20  ahd
 *    Allow raw message transmission for POP messages
 *    Make all receive errors fatal
 *
 *    Revision 1.11  1998/03/08 04:50:04  ahd
 *    Close socket after read errors
 *
 *    Revision 1.10  1998/03/06 06:51:28  ahd
 *    Add commands to make Netscape happy
 *
 *    Revision 1.9  1998/03/03 03:51:53  ahd
 *    Routines to handle messages within a POP3 mailbox
 *
 *    Revision 1.8  1998/03/01 19:42:17  ahd
 *    First compiling POP3 server which accepts user id/password
 *
 *    Revision 1.7  1997/11/29 13:03:13  ahd
 *    Clean up single client (hot handle) mode for OS/2, including correct
 *    network initialization, use unique client id (pid), and invoke all
 *    routines needed in main client loop.
 *
 *    Revision 1.6  1997/11/28 23:11:38  ahd
 *    Additional SMTP auditing, normalize formatting, more OS/2 SMTP fixes
 *
 *    Revision 1.5  1997/11/28 04:52:10  ahd
 *    Initial UUSMTPD OS/2 support
 *
 *    Revision 1.4  1997/11/26 03:34:11  ahd
 *    Correct SMTP timeouts, break out protocol from rest of daemon
 *
 *    Revision 1.3  1997/11/25 05:05:06  ahd
 *    More robust SMTP daemon
 *
 *    Revision 1.2  1997/11/24 02:52:26  ahd
 *    First working SMTP daemon which delivers mail
 *
 *    Revision 1.1  1997/11/21 18:15:18  ahd
 *    Command processing stub SMTP daemon
 *
 *    Revision 1.1  1997/06/03 03:25:31  ahd
 *    Initial revision
 *
 *    Revision 1.1  1997/05/20 03:55:46  ahd
 *    Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                         Standard includes                          */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <limits.h>
#include <ctype.h>

#include "smtpnetw.h"
#include "catcher.h"
#include "ssleep.h"
#include "smtpverb.h"
#include "memstr.h"

#define MAX_BUFFER_SIZE (1024 * 64)

/*--------------------------------------------------------------------*/
/*                      Global defines/variables                      */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtpnetw.c 1.24 1999/01/08 02:21:05 ahd Exp $");

#define MINUTE(seconds) ((seconds)*60)

#if defined(__OS2__)
KWBoolean winsockActive = KWFalse;  /* Initialized here -- <not> in catcher.c
                                     No need for catcher -- no WSACleanup() of
                                     OS/2 sockets required                  */
#else
extern KWBoolean winsockActive;                 /* Initialized in catcher.c  */
#endif

static const char crlf[] = "\r\n";

/*--------------------------------------------------------------------*/
/*                          Local prototypes                          */
/*--------------------------------------------------------------------*/

static size_t
SMTPRead(SMTPClient *client);

static size_t
SMTPWrite(SMTPClient *client,
          const char UUFAR *data,
          unsigned int len);

static void
SMTPBurpBuffer(SMTPClient *client);

static KWBoolean
isFatalSocketError(int err);

#if !defined(__OS2__)
void AtWinsockExit(void);
#endif

/*--------------------------------------------------------------------*/
/*       g e t L i n e B r e a k                                      */
/*                                                                    */
/*       Determine where end of next line is                          */
/*--------------------------------------------------------------------*/

static char
*getLineBreak( SMTPBuffer *sb )
{
   size_t offset;
   static const char mName[] = "getLineBreak";

   if ( sb->next == NULL)
      return NULL;

   offset = sb->next - sb->buffer;

#ifdef UDEBUG
   if (offset > sb->used)
   {
      printmsg(0,"%s: Attempted to scan %ld bytes in a "
                  "buffer only %ld bytes long",
                  mName,
                  (long) offset,
                  (long) sb->used);
      panic();
   }
#endif

   return memstr(sb->next, crlf, sb->used - offset);

} /* lineBreak */

/*--------------------------------------------------------------------*/
/*       S M T P G e t L i n e                                        */
/*                                                                    */
/*       Read an SMTP command from our internal buffer                */
/*--------------------------------------------------------------------*/

KWBoolean
SMTPGetLine(SMTPClient *client)
{
   static const char mName[] = "SMTPGetLine";
   char *lineBreak;

#ifdef UDEBUG2

#define ignoredBytes(x) (((x).next == NULL) ? 0 : (x).next - (x).buffer)

   if ( debuglevel >= 8 )
      printmsg(8, "%s: Client %d in mode 0x%04x "
                   "with %d of possible %d bytes buffered (%d ignored)",
                   mName,
                   getClientSequence(client),
                   getClientMode(client),
                   client->receive.used,
                   client->receive.allocated,
                   ignored(client->receive));
#endif

/*--------------------------------------------------------------------*/
/*                   Handle previously signaled EOF                   */
/*--------------------------------------------------------------------*/

   if (isClientEOF(client) && (client->receive.next == NULL))
   {
      printmsg(0, "%s: client %d is out of data (EOF)",
                   mName,
                   getClientSequence(client));

      client->receive.line = client->receive.buffer;
      client->receive.line[ 0 ] = '\0';
      setClientMode(client, SM_ABORT);
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*       Read more data if we have some and need it,                  */
/*--------------------------------------------------------------------*/

   lineBreak = getLineBreak(&client->receive);

   if (getClientReady(client) && (lineBreak == NULL))
   {
      if (client->stalledReads)     /* Improve response time ...     */
         client->stalledReads--;

      SMTPBurpBuffer(client);
      SMTPRead(client);
      lineBreak = getLineBreak(&client->receive);
   }

/*--------------------------------------------------------------------*/
/*              If no data available, return immediately              */
/*--------------------------------------------------------------------*/

   if (lineBreak == NULL)
   {

      if (isClientEOF(client))
      {
         static const char quit[] = "quit";
         static const size_t quitLength = sizeof quit - 1;

         if ((client->receive.next == client->receive.buffer) &&
             (client->receive.used == quitLength) &&
             equalni(client->receive.buffer, "quit", quitLength))
         {
            printmsg(8,"%s: Applying CR/LF after Netscape %s/EOF",
                     mName,
                     quit);
            lineBreak = client->receive.buffer + quitLength;
            client->receive.used += 2;
         }
         else {
            printmsg(0, "%s: Client %d Terminated unexpectedly without QUIT",
                       mName,
                       getClientSequence(client));
            client->receive.line = NULL;

            /* Abort client immediately */
            setClientMode(client, SM_ABORT);
            return KWTrue;
         }
      }
      else if (client->receive.used < client->receive.allocated)
      {
         printmsg(2, "%s: Client %d Input buffer "
                      "(%d bytes) waiting for data.",
                      mName,
                      getClientSequence(client),
                      client->receive.used);

         /* Sleep client for few secs  */
         setClientIgnore(client, (time_t) ++client->stalledReads);

         /* Don't process command yet  */
         return KWFalse;

      } /* if (client->receive.used < client->receive.allocated) */
      else {

        printmsg(0, "%d <<< %.125s",
                     getClientSequence(client),
                     client->receive.next);
        printmsg(0, "%s: Client %d Input buffer (%d bytes) overrun.",
                     mName,
                     getClientSequence(client),
                     client->receive.used);

        client->receive.lineLength = client->receive.used - 1;

        /* Don't run off end of the buffer */
        client->receive.buffer[client->receive.used - 1] = '\0';

        /* Abort client immediately     */
        setClientMode(client, SM_ABORT);
        return KWTrue;

      } /* else */

   } /* if (lineBreak == NULL) */

   client->receive.line = client->receive.next;
   client->receive.lineLength = lineBreak - client->receive.line;

   /* Terminate the command line and step past the CR/LF */
   *(lineBreak++) = '\0';
   *(lineBreak++) = '\0';

   /* Remember where our next line starts, if any */
   client->receive.next = lineBreak;

   if (client->receive.next == (client->receive.buffer +
                                client->receive.used))
   {
      client->receive.next = NULL;

#ifdef UDEBUG2
      if (debuglevel >= 6)
         printmsg(6,"%s: Last line in buffer %p (%d bytes) at %p: %.125s",
                  mName,
                  client->receive.buffer,
                  client->receive.allocated,
                  client->receive.line,
                  client->receive.line );
#endif
   }

/*--------------------------------------------------------------------*/
/*           Locate start of input line if not in data mode           */
/*--------------------------------------------------------------------*/

   if (isClientFlag(client, SF_NO_TOKENIZE))
   {
      clearClientFlag(client, SF_NO_TOKENIZE);

      printmsg(5,"%d <<< %.125s",
               getClientSequence(client),
               client->receive.line );
   }
   else {

      while(isspace(*(client->receive.line)))
         client->receive.line++;

      if (*(client->receive.line) == '\0')
      {
         printmsg(0, "%d <<<   (empty line with %d characters)",
                      getClientSequence(client),
                      client->receive.lineLength);
         client->receive.lineLength= 0;
         setClientIgnore(client, 2);      /* Make client wait */

         /* Ignore input line */
         return KWFalse;
      }

      /* Recompute (perhaps updated) line length */
      client->receive.lineLength = lineBreak - client->receive.line - 2;

      if ( equalni(client->receive.line, "pass", 4))
         printmsg(2,"%d <<< %.4s xxxxxxxx",
                  getClientSequence(client),
                  client->receive.line );
      else
         printmsg(2,"%d <<< %.125s",
                    getClientSequence(client),
                    client->receive.line );

   } /* else */

   return KWTrue;

} /* SMTPGetLine */

/*--------------------------------------------------------------------*/
/*    I n i t W i n s o c k                                           */
/*                                                                    */
/*    Start the Windows sockets DLL                                   */
/*--------------------------------------------------------------------*/

KWBoolean
InitWinsock(void)
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

} /* InitWinsock */

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

#ifdef __TURBOC__
#pragma argsused
#endif

SOCKET
openMaster(const char *name)
{
   static const char mName[] = "masterOpen";

   SOCKET pollingSock;
   SOCKADDR_IN sin;
   LPSERVENT pse;
   int sockopt = 1;

   if (!InitWinsock())              /* Initialize library?           */
      return INVALID_SOCKET;        /* No --> Report error           */

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

   if (listen(pollingSock, 2) == SOCKET_ERROR)
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
   if (debuglevel > NETDEBUG)
   {
      static int option[] = { SO_SNDBUF, SO_RCVBUF };
      static int optionCount = (sizeof option / sizeof option[0]);
      int subscript;

      printmsg(NETDEBUG, "%s: Printing %d options for socket %d",
                         mName,
                         optionCount,
                         connectedSock );

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

static size_t
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

static size_t
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
      return client->receive.used;

   if (received == 0)
   {
      client->endOfTransmission = KWTrue;
      printmsg(0, "%s: client %d EOF on recv(%ld,%p,%d,%d)",
                  mName,
                  getClientSequence(client),
                  (long) getClientHandle(client),
                  client->receive.buffer + client->receive.used,
                  (int) (client->receive.allocated - client->receive.used),
                  0);
   }

   }
   else {
      incrementClientBytesRead(client, (size_t) received);
      client->receive.used += (size_t) received;

      if (client->receive.next == NULL)
         client->receive.next = client->receive.buffer;
   }

   return client->receive.used;

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


   handle = -1;

} /* closeSocket */

/*--------------------------------------------------------------------*/
/*       s e l e c t R e a d y S o c k e t s                          */
/*                                                                    */
/*       Perform select to determine what sockets are ready,          */
/*       waiting if needed                                            */
/*--------------------------------------------------------------------*/

KWBoolean
selectReadySockets(SMTPClient *current)
{
   static const char mName[] = "flagReadySockets";

/*--------------------------------------------------------------------*/
/*              For batch SMTP, sockets are always ready              */
/*--------------------------------------------------------------------*/

   do {

      assertSMTP(current);

      if (isClientValid(current))
      {
         setClientReady(current, KWTrue);
         setClientProcess(current, KWTrue);
      }

      current = current->next;

   } while(current);

   return KWTrue;

} /* flagReadySockets */