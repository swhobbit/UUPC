/*--------------------------------------------------------------------*/
/*       s m t p n e t w . c                                          */
/*                                                                    */
/*       SMTP verb parser for UUPC/extended                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1998 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpnetw.c 1.12 1998/03/08 23:10:20 ahd Exp $
 *
 *    $Log: smtpnetw.c $
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

#define MAX_BUFFER_SIZE (1024 * 64)

/*--------------------------------------------------------------------*/
/*                      Global defines/variables                      */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtpnetw.c 1.12 1998/03/08 23:10:20 ahd Exp $");

currentfile();

#define MINUTE(seconds) (seconds*60)

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
/*       S M T P G e t L i n e                                        */
/*                                                                    */
/*       Read an SMTP command from our internal buffer                */
/*--------------------------------------------------------------------*/

KWBoolean
SMTPGetLine(SMTPClient *client)
{
   static const char mName[] = "SMTPGetLine";
   size_t column;

   printmsg(5, "%s: entered for client %d in mode 0x%04x "
                "with %d bytes available",
                mName,
                getClientSequence(client),
                getClientMode(client),
                client->receive.used);

   SMTPBurpBuffer(client);

/*--------------------------------------------------------------------*/
/*                   Handle previously signaled EOF                   */
/*--------------------------------------------------------------------*/

   if (isClientEOF(client) && ! client->receive.used)
   {
      printmsg(0, "%s: client %d is out of data (EOF)",
                   mName,
                   getClientSequence(client));
      client->receive.data[ 0 ] = '\0';
      setClientMode(client, SM_ABORT);
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*       Read more data if we have some and need it, need being       */
/*       defined as we have used 75% of the buffer or no CR/LF        */
/*       was found during our last pass.                              */
/*--------------------------------------------------------------------*/

   if (getClientReady(client) &&
       ((client->stalledReads > 0) ||
       (client->receive.used < (client->receive.length / 4))))
   {
      if (client->stalledReads)     /* Improve response time ...     */
         client->stalledReads--;
      SMTPRead(client);
   }

/*--------------------------------------------------------------------*/
/*           Locate start of input line if not in data mode           */
/*--------------------------------------------------------------------*/

   if (getClientMode(client) != SM_DATA)
   {
      for(column = 0;
           column < client->receive.used;
           column ++)
      {
            if (! isspace(client->receive.data[column]))
               break;
      }

      if (column == client->receive.used)
      {
         printmsg(2, "%d <<<   (empty line with %d characters)",
                      getClientSequence(client),
                      client->receive.used);
         client->receive.used = 0;
         client->receive.data[ 0 ] = '\0';
         setClientIgnore(client, 2);      /* Make client wait */
         return KWFalse;                  /* Ignore input line */

      }
      else if (column > 0)
      {

         client->receive.used -= column;
         memmove(client->receive.data,
                  client->receive.data + column,
                  client->receive.used);
      }

      /* Silly hack to handle NETSPACE being lazy about
         terminating QUIT commands                       */
      if ((client->receive.used > 3) &&
           (client->receive.used < 6) &&
           equalni(client->receive.data, "QUIT", 4))
      {
         printmsg(1, "%s: Client %d requires CR/LF after QUIT",
                     mName,
                     getClientSequence(client));
         strcpy(client->receive.data + 4, crlf);
         client->receive.used = 6;
      }

   } /* if (getClientMode(client) != SM_DATA) */

/*--------------------------------------------------------------------*/
/*    Locate the end of the input line; we deliberately parse past    */
/*    embedded nulls ('\0') lest some idiot sent us binary data       */
/*--------------------------------------------------------------------*/

   for(column = 0;
        column < client->receive.used;
        column ++)
   {
      /* After we have at least two characters, check for CR/LF pair */
      if (column &&
           ! memcmp(client->receive.data + column - 1,
                     crlf,
                     2))
      {
         client->receive.data[column-1] = '\0';
         printmsg((int) ((getClientMode(client) == SM_DATA) ? 8 : 2),
                      "%d <<< %.75s",
                      getClientSequence(client),
                      client->receive.data);
         incrementClientLinesRead(client);
         client->receive.parsed = column + 1;
                                    /* Incr because count, not
                                       subscript for this            */
         return KWTrue;

      } /* if */

   } /* for */

   if (isClientEOF(client))
   {
      printmsg(0, "%s: Client %d Terminated unexpectedly without QUIT",
                 mName,
                 getClientSequence(client));
      client->receive.data[ 0 ] = '\0';
      setClientMode(client, SM_ABORT);/* Abort client immediately     */
      return KWTrue;                 /* Process the abort immediately */
   }

/*--------------------------------------------------------------------*/
/*       We did not find the end of the command; this is an error     */
/*       only if we are also out of buffer space.                     */
/*--------------------------------------------------------------------*/

   if (client->receive.used < client->receive.length)
   {
      client->receive.parsed = 0;      /* Flag buffer unprocessed    */
      client->receive.data[ client->receive.used ] = '\0';

      printmsg(4, "%s: Client %d Input buffer "
                   "(%d bytes) waiting for data.",
                   mName,
                   getClientSequence(client),
                   client->receive.used);
      setClientIgnore(client, (time_t) ++client->stalledReads);
                                       /* Sleep client for few secs  */
      return KWFalse;                  /* Don't process command now  */

   } /* if (client->received.used < client->receive.length) */
   else {

     printmsg(0, "%d <<< %.75s",
                  getClientSequence(client),
                  client->receive.data);
     printmsg(0, "%s: Client %d Input buffer (%d bytes) overrun.",
                   mName,
                   getClientSequence(client),
                   client->receive.used);

     client->receive.parsed = client->receive.used;
     client->receive.data[ client->receive.used - 1 ] = '\0';
                                    /* Don't run off the buffer      */
     setClientMode(client, SM_ABORT);/* Abort client immediately     */
     return KWTrue;                 /* Process the abort immediately */

   } /* else */

} /* SMTPGetLine */

/*--------------------------------------------------------------------*/
/*       S M T P R e s p o n s e                                      */
/*                                                                    */
/*       Send a response to a remote client                           */
/*--------------------------------------------------------------------*/

KWBoolean
SMTPResponse(SMTPClient *client, int code, const char *text)
{

   char buf[BUFSIZ];
   size_t totalLength;
   KWBoolean buffered = KWFalse;
   int printLevel = 2;

   switch(code)
   {
         case 0:
            strcpy(buf, "??? ");
            printLevel = 0;
            break;

         case PR_TEXT:
            *buf = '\0';
            printLevel = 8;
            break;

         case PR_DATA:
            *buf = '\0';
            break;

         case PR_OK_GENERIC:
            strcpy(buf, "+OK ");
            break;

         case PR_ERROR_GENERIC:
            printLevel = 0;
            /* Fall through to warning */

         case PR_ERROR_WARNING:
            strcpy(buf, "-ERR ");
            break;

         default:
            /* Numeric codes for SMTP */
            switch( code / 100 )
            {
               case 4:
               case 5:
                  printLevel = 0;
                  break;

               default:
                  break;
            }

            sprintf(buf, "%03.3d%c",
                          (code < 0) ? - code : code,
                          (code < 0) ? '-' : ' ');
            break;

   } /* switch(code) */

   if ( printLevel >= debuglevel )
   {
      printmsg(printLevel,"%d >>> %s%.75s",
                          getClientSequence(client),
                          buf,
                          text);
   }

   totalLength = strlen(buf) + strlen(text);
   incrementClientBytesWritten(client, totalLength);
   incrementClientLinesWritten(client);

/*--------------------------------------------------------------------*/
/*       Special case for raw text with CR/LF already appended        */
/*--------------------------------------------------------------------*/

   if (code == PR_TEXT)
   {
      if(!SMTPWrite(client, text, totalLength))
      {
         printmsg(0,"Error sending response text to remote host: %s%.75s",
                    buf,
                    text);
         return KWFalse;
      }

      return KWTrue;

   } /* if (code == PR_TEXT) */

/*--------------------------------------------------------------------*/
/*       If all three parts of the message fit, pack it into one      */
/*       buffer so we only call the network write once to send        */
/*       one packet.                                                  */
/*--------------------------------------------------------------------*/

   totalLength += strlen(crlf);
   incrementClientBytesWritten(client, strlen(crlf));

   if (totalLength < sizeof buf)
   {
      strcat(buf, text);
      strcat(buf, crlf);
      buffered = KWTrue;
   }

   if(!SMTPWrite(client, buf, strlen(buf)))
   {
      printmsg(0,"Error sending response code to remote host: %.4s %.75s",
                  buf,
                  text);
      return KWFalse;
   }

   if (buffered)                    /* All info written from buffer? */
      return KWTrue;                /* Yes --> Report success        */

   if(!SMTPWrite(client, text, strlen(text)))
   {
      printmsg(0,"Error sending response text to remote host: %s%.75s",
                 buf,
                 text);
      return KWFalse;
   }

   if(!SMTPWrite(client, crlf, strlen(crlf)))
   {
      printmsg(0, "Error sending CR/LF to remote host: %s%.75s",
                  buf,
                  text);
      return KWFalse;
   }

   return KWTrue;

} /* SMTPResponse */

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
      case SM_DELETE_PENDING:     return 0;
      case SM_MASTER:             return LONG_MAX;
      case SM_CONNECTED:          return 0;
      case SM_UNGREETED:          return MINUTE(5);
      case SM_IDLE:               return MINUTE(10);
      case SM_ADDR_FIRST:         return MINUTE(5);
      case SM_ADDR_SECOND:        return MINUTE(5);
      case SM_DATA:               return MINUTE(15);
      case SM_ABORT:              return 0;
      case SM_TIMEOUT:            return 0;
      case SM_EXITING:            return 0;

      default:                    return MINUTE(1);

   } /* switch(mode) */

} /* getModeTimeout */

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
         (char UUFAR *)&sockopt, sizeof(int)) == SOCKET_ERROR)
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
   static const char mName[] = "slaveOpen";

   SOCKET connectedSock = accept(pollingSock, NULL, NULL);

   if (connectedSock == INVALID_SOCKET)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "%s: could not accept a connection", mName);
      printWSerror("accept", wsErr);
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

   status = send(getClientHandle(client),
                 (char UUFAR *)data,
                 (int) len,
                 0);

   if (status == SOCKET_ERROR)
   {
      int err;

      err = WSAGetLastError();
      printmsg(0, "%s: Error sending data to socket", mName);
      printWSerror("send", err);

      if (isFatalSocketError(err))
      {
         shutdown(getClientHandle(client),
                  2);               /* Fail both reads and writes   */
      }

      /* All write errors are treated as fatal, close the socket */
      closeSocket( getClientHandle(client) );
      setClientHandle( client, INVALID_SOCKET );
      return 0;
   }

   if (status < (int)len)     /* Breaks if len > 32K, which is unlikely */
      printmsg(0,"%s: Write to network failed.", mName);

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

/*--------------------------------------------------------------------*/
/*               Reset flag which drives our invocation               */
/*--------------------------------------------------------------------*/

   setClientReady(client, KWFalse);

/*--------------------------------------------------------------------*/
/*                If no more data from client, return                 */
/*--------------------------------------------------------------------*/

   if (client->endOfTransmission)
      return client->receive.used;

/*--------------------------------------------------------------------*/
/*                 Make sure our buffer is big enough                 */
/*--------------------------------------------------------------------*/

   if (client->receive.used >= client->receive.length)
   {
      if (client->receive.length < MAX_BUFFER_SIZE)
      {
         printmsg(2, "%s: Client %d buffer size doubled to %d bytes",
                    mName,
                    getClientSequence(client),
                    client->receive.length);
         client->receive.length *= 2;
         client->receive.data =
                       realloc(client->receive.data,
                                 client->receive.length);
         checkref(client->receive.data);

      } /* if (client->receive.length < MAX_BUFFER_SIZE) */
      else {
          printmsg(0, "%s: Client %d overran of input buffer %d,"
                      " truncated.",
                      mName,
                      getClientSequence(client),
                      client->receive.length);
          return client->receive.used;

      } /* else */

   } /* if (client->receive.used >= client->receive.length) */

/*--------------------------------------------------------------------*/
/*                  Actually get our next data read                   */
/*--------------------------------------------------------------------*/

   received = recv(getClientHandle(client),
                   client->receive.data + client->receive.used,
                   (int) (client->receive.length - client->receive.used),
                   0);

   if (received == 0)
   {
      client->endOfTransmission = KWTrue;
      printmsg(0, "%s: client %d EOF on recv()",
                  mName,
                  getClientSequence(client));
   }
   else if (received == SOCKET_ERROR)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "%s: client %d recv() failed",
                  mName,
                  getClientSequence(client));
      printWSerror("recv", wsErr);

      /* All errors are treated as fatal, close the socket */
      closeSocket( getClientHandle(client) );
      setClientHandle( client, INVALID_SOCKET );
      return 0;

   }
   else {
      incrementClientBytesRead(client, (size_t) received);
      client->receive.used += (size_t) received;
   }

   return client->receive.used;

} /* SMTPRead */

/*--------------------------------------------------------------------*/
/*      i s F a t a l S o c k e t E r r o r                           */
/*                                                                    */
/*      Determine if an error is a show stopped                       */
/*--------------------------------------------------------------------*/

static KWBoolean
isFatalSocketError(int err)
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

} /* isFatalSocketError */

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
/*       S M T P B u r p B u f f e r                                  */
/*                                                                    */
/*       Discard already processed data from the input buffer         */
/*--------------------------------------------------------------------*/

static void
SMTPBurpBuffer(SMTPClient *client)
{

   static const char mName[] = "SMTPBurpBuffer";

/*--------------------------------------------------------------------*/
/*                 Verify the status of the input buffer              */
/*--------------------------------------------------------------------*/

   if (client->receive.length < client->receive.used)
   {
         printmsg(0, "%s: Client has used more bytes (%d) "
                     "than buffer bytes allocated (%d)",
                    mName,
                    getClientSequence(client),
                    client->receive.used,
                    client->receive.length);

      panic();
   }

   if (client->receive.used < client->receive.parsed)
   {
         printmsg(0, "%s: Client has parsed more bytes (%d) "
                     "than bytes in use (%d)",
                    mName,
                    getClientSequence(client),
                    client->receive.parsed,
                    client->receive.used);

      panic();
   }

/*--------------------------------------------------------------------*/
/*     Discard any data we have already processed from the client     */
/*--------------------------------------------------------------------*/

   if (client->receive.parsed > 0)
   {
      client->receive.used -= client->receive.parsed;

      if (client->receive.used > 0)
         memmove(client->receive.data,
                  client->receive.data + client->receive.parsed,
                  client->receive.used);
      client->receive.parsed = 0;

   } /* if (client->receive.parsed > 0) */

} /* SMTPBurpBuffer */

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
      if ( timeoutPeriod.tv_sec > 0 )
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
         ssleep(10);
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
               (long) timeoutPeriod.tv_sec );
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

   printmsg(nReady ? 4 : 8, "%s: %d of %d (out of %d) sockets were ready.",
             mName,
             nReady,
             nSelected,
             nTotal);

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
