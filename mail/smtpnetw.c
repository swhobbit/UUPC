/*--------------------------------------------------------------------*/
/*       s m t p n e t w . c                                          */
/*                                                                    */
/*       SMTP verb parser for UUPC/extended                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1997 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpnetw.c 1.2 1997/11/24 02:52:26 ahd Exp $
 *
 *    $Log: smtpnetw.c $
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

#include "smtpnetw.h"
#include "catcher.h"
#include "ssleep.h"

/*--------------------------------------------------------------------*/
/*                      Global defines/variables                      */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtpnetw.c 1.2 1997/11/24 02:52:26 ahd Exp $");

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

static int
SMTPRead( SMTPClient *client );

static int
SMTPWrite(SMTPClient *client,
          const char UUFAR *data,
          unsigned int len);

static void
SMTPBurpBuffer( SMTPClient *client );

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
SMTPGetLine( SMTPClient *client )
{
   static const char mName[] = "SMTPGetLine";
   int column;

   printmsg( 5, "%s: entered for client %d in mode 0x%04x "
                "with %d bytes available",
                mName,
                getClientSequence( client ),
                getClientMode( client ),
                client->receive.used );

   SMTPBurpBuffer( client );

/*--------------------------------------------------------------------*/
/*                   Handle previously signaled EOF                   */
/*--------------------------------------------------------------------*/

   if ( isClientEOF( client ) && ! client->receive.used )
   {
      printmsg( 0, "%s: client %d is out of data (EOF)",
                   mName,
                   getClientSequence( client ));
      client->receive.data[ 0 ] = '\0';
      setClientMode( client, SM_ABORT );
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*                    Read more data if we need it                    */
/*--------------------------------------------------------------------*/

   if ( getClientReady( client ))
   {
      if ( client->stalledReads )   /* Improve response time ...     */
         client->stalledReads--;
      SMTPRead( client );
   }

/*--------------------------------------------------------------------*/
/*           Locate start of input line if not in data mode           */
/*--------------------------------------------------------------------*/

   if ( getClientMode( client ) != SM_DATA )
   {
      for( column = 0;
           column < client->receive.used;
           column ++ )
      {
            if ( ! isspace( client->receive.data[column] ))
               break;
      }

      if ( column == client->receive.used )
      {
         printmsg( 2, "%d <<<  (empty line with %d characters)",
                      getClientSequence(client),
                      client->receive.used);
         client->receive.used = 0;
         client->receive.data[ 0 ] = '\0';
         setClientIgnore( client, 2 );    /* Make client wait */
         return KWFalse;                  /* Ignore input line */

      }
      else if ( column > 0 )
      {

         client->receive.used -= column;
         memmove( client->receive.data,
                  client->receive.data + column,
                  client->receive.used );
      }

   } /* if ( getClientMode( client ) != SM_DATA ) */

/*--------------------------------------------------------------------*/
/*    Locate the end of the input line; we deliberately parse past    */
/*    embedded nulls ('\0') lest some idiot sent us binary data       */
/*--------------------------------------------------------------------*/

   for( column = 0;
        column < client->receive.used;
        column ++ )
   {
      /* After we have at least two characters, check for CR/LF pair */
      if ( column &&
           ! memcmp( client->receive.data + column - 1,
                     crlf,
                     2 ))
      {
         client->receive.data[column-1] = '\0';
         printmsg( ( getClientMode( client ) == SM_DATA ) ? 8 : 2,
                      "%d <<< %.75s",
                      getClientSequence(client),
                      client->receive.data );
         incrementClientLinesRead( client );
         client->receive.parsed = column + 1;
                                    /* Incr because count, not
                                       subscript for this            */
         return KWTrue;

      } /* if */

   } /* for */

   if ( isClientEOF( client ))
   {
      printmsg( 0, "%s: Client %d Terminated unexpectedly",
                 mName,
                 getClientSequence( client ) );
      client->receive.data[ 0 ] = '\0';
      setClientMode( client, SM_ABORT );/* Abort client immediately   */
      return KWTrue;                 /* Process the abort immediately */
   }

/*--------------------------------------------------------------------*/
/*       We did not find the end of the command; this is an error     */
/*       only if we are also out of buffer space.                     */
/*--------------------------------------------------------------------*/

   if ( client->receive.used < client->receive.length)
   {
      client->receive.parsed = 0;      /* Flag buffer unprocessed    */
      client->receive.data[ client->receive.used ] = '\0';

      printmsg( 4, "%s: Client %d Input buffer "
                   "(%d bytes) waiting for data.",
                   mName,
                   getClientSequence( client ),
                   client->receive.used );
      setClientIgnore( client, ++client->stalledReads );
                                       /* Sleep client for few secs  */
      return KWFalse;                  /* Don't process command now  */

   } /* if ( client->received.used < client->receive.length) */
   else {

     printmsg( 0, "%d <<< %.75s",
                  getClientSequence(client),
                  client->receive.data );
     printmsg( 0, "%s: Client %d Input buffer (%d bytes) overrun.",
                   mName,
                   getClientSequence( client ),
                   client->receive.used );

     client->receive.parsed = client->receive.used;
     client->receive.data[ client->receive.used - 1 ] = '\0';
                                    /* Don't run off the buffer      */
     setClientMode( client, SM_ABORT );/* Abort client immediately   */
     return KWTrue;                 /* Process the abort immediately */

   } /* else */

} /* SMTPGetLine */

/*--------------------------------------------------------------------*/
/*       S M T P R e s p o n s e                                      */
/*                                                                    */
/*       Send a response to a remote client                           */
/*--------------------------------------------------------------------*/

KWBoolean
SMTPResponse( SMTPClient *client, int code, const char *text )
{

   char buf[BUFSIZ];
   size_t totalLength;
   KWBoolean buffered = KWFalse;

   if ( code )
      sprintf( buf, "%03.3d%c", code < 0 ? - code : code,
                         code < 0 ? '-' : ' ' );
   else
      strcpy(buf, "??? " );

   printmsg( 2,"%d >>> %s%.75s", getClientSequence(client), buf, text);

   incrementClientLinesWritten( client );
   incrementClientBytesWritten( client, totalLength );

   totalLength = strlen( buf ) + strlen( text ) + strlen( crlf );

/*--------------------------------------------------------------------*/
/*       If all three parts of the message fit, pack it into one      */
/*       buffer so we only call the network write once and send       */
/*       one packet.                                                  */
/*--------------------------------------------------------------------*/

   if ( totalLength < sizeof buf )
   {
      strcat( buf, text );
      strcat( buf, crlf );
      buffered = KWTrue;
   }

   if( !SMTPWrite( client, buf, strlen(buf)) )
   {
      printmsg(0,"Error sending response code to remote host: %.4s %.75s",
                  buf,
                  text );
      return KWFalse;
   }

   if ( buffered )                  /* All info written from buffer? */
      return KWTrue;                /* Yes --> Report success        */

   if( !SMTPWrite( client, text, strlen(text)) )
   {
      printmsg(0,"Error sending response text to remote host: %s%.75s",
                 buf,
                 text );
      return KWFalse;
   }

   if( !SMTPWrite( client, crlf, strlen( crlf )) )
   {
      printmsg(0, "Error sending CR/LF to remote host: %s%.75s",
                  buf,
                  text );
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
getModeTimeout( SMTPMode mode )
{

   switch( mode )
   {
      case SM_INVALID:     return 0;
      case SM_MASTER:      return LONG_MAX;
      case SM_CONNECTED:   return 0;
      case SM_UNGREETED:   return MINUTE(5);
      case SM_IDLE:        return MINUTE(10);
      case SM_ADDR_FIRST:  return MINUTE(5);
      case SM_ADDR_SECOND: return MINUTE(5);
      case SM_DATA:        return MINUTE(15);
      case SM_ABORT:       return 0;
      case SM_TIMEOUT:     return 0;
      case SM_EXITING:     return 0;

      default:             return MINUTE(1);

   } /* switch( mode ) */

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
openMaster(const char *name )
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

   printmsg(NETDEBUG, "%s: determining port", mName);

   if ( strlen( name ))
      sin.sin_port = htons((u_short) atoi(name));
   else if ( (pse = getservbyname(UUCP_SERVICE, "tcp")) == NULL )
   {
      int wsErr = WSAGetLastError();

      sin.sin_port = htons(UUCP_PORT);
      printWSerror("getservbyname", wsErr);
      printmsg(0, "%s: cannot locate service %s, using port %d",
                  mName,
                  UUCP_SERVICE,
                  (int)ntohs(sin.sin_port));
   }
   else
      sin.sin_port = pse->s_port;

   sin.sin_family = AF_INET;     /* A internet socket, of course     */
   sin.sin_addr.s_addr = 0;      /* Listen on all interfaces         */

/*--------------------------------------------------------------------*/
/*                     Create and bind TCP socket                     */
/*--------------------------------------------------------------------*/

   printmsg(NETDEBUG + 1, "%s: doing socket()", mName );

   pollingSock = socket( AF_INET, SOCK_STREAM, 0);

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
      closeSocket( pollingSock );
      return INVALID_SOCKET;        /* report failure            */
   }

   printmsg(NETDEBUG + 1, "%s: doing setsockopt()", mName);

   if (setsockopt( pollingSock, SOL_SOCKET, SO_REUSEADDR,
         (char UUFAR *)&sockopt, sizeof(int)) == SOCKET_ERROR)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "%s: setsockopt() failed", mName);
      printWSerror("setsockopt", wsErr);
      closeSocket( pollingSock );
      return INVALID_SOCKET;
   }

   printmsg(NETDEBUG, "%s: doing listen()", mName);

   if (listen(pollingSock, 2) == SOCKET_ERROR)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "%s: listen(pollingSock) failed", mName);
      printWSerror("listen", wsErr);
      closeSocket( pollingSock );
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

static int
SMTPWrite(SMTPClient *client,
          const char UUFAR *data,
          unsigned int len)
{
   static const char mName[] = "SMTPWrite";
   int status;

   status = send(client->handle, (char UUFAR *)data, (int) len, 0);

   if (status == SOCKET_ERROR)
   {
      int err;

      err = WSAGetLastError();
      printmsg(0, "%s: Error sending data to socket", mName);
      printWSerror("send", err);

      if (isFatalSocketError(err))
      {
         shutdown(client->handle, 2);  /* Fail both reads and writes   */
      }
      return 0;
   }

   if (status < (int)len)     /* Breaks if len > 32K, which is unlikely */
   {
      printmsg(0,"%s: Write to network failed.", mName);
      return status;
   }
/*--------------------------------------------------------------------*/
/*              Return byte count transmitted to caller               */
/*--------------------------------------------------------------------*/

   return (int) len;

} /* SMTPWrite */

/*--------------------------------------------------------------------*/
/*       S M T P R e a d                                              */
/*                                                                    */
/*       Perform a read off the network to (perhaps) fill in the      */
/*       supplied client read buffer; should be called only after     */
/*       select determines client is ready.                           */
/*--------------------------------------------------------------------*/

static int
SMTPRead( SMTPClient *client )
{
   static const char mName[] = "SMTPRead";
   int received;

/*--------------------------------------------------------------------*/
/*               Reset flag which drives our invocation               */
/*--------------------------------------------------------------------*/

   setClientReady( client, KWFalse );

/*--------------------------------------------------------------------*/
/*                If no more data from client, return                 */
/*--------------------------------------------------------------------*/

   if ( client->endOfTransmission )
      return client->receive.used;

/*--------------------------------------------------------------------*/
/*                 Make sure our buffer is big enough                 */
/*--------------------------------------------------------------------*/

   if ( client->receive.used >= client->receive.length )
   {
      if (client->receive.length < (16*1024))
      {
         printmsg(2, "%s: Client %d buffer size doubled to %d bytes",
                    mName,
                    getClientSequence( client ),
                    client->receive.length );
         client->receive.length *= 2;
         client->receive.data =
                       realloc( client->receive.data,
                                 client->receive.length );
         checkref( client->receive.data );

      } /* if (client->receive.length < (16*1024)) */
      else {
          printmsg(0, "%s: Client %d overran of input buffer %d,"
                      " truncated.",
                      mName,
                      getClientSequence( client ),
                      client->receive.length );
          return client->receive.used;

      } /* else */

   } /* if ( client->receive.used >= client->receive.length ) */

/*--------------------------------------------------------------------*/
/*                  Actually get our next data read                   */
/*--------------------------------------------------------------------*/

   received = recv(client->handle,
                   client->receive.data +
                           client->receive.used,
                   client->receive.length -
                           client->receive.used,
                   0);

   if ( received == 0 )
   {
      client->endOfTransmission = KWTrue;
      printmsg(0, "%s: client %d EOF on recv()",
                  mName,
                  getClientSequence( client ));

      /* Stupid hack for netscape, which seems to skip the final CR/LF */
      if (( client->receive.used >= 2 ) &&
          ( client->receive.used < (client->receive.length -
                                    (int) sizeof crlf)) &&
            ! equaln( client->receive.data + client->receive.used - 2,
                     crlf,
                     2 ))
      {

#ifdef UDEBUG
         client->receive.data[client->receive.used] = '\0';
         printmsg( 5, "%s: Client %d needed final CR/LF at EOF: \"%.75s\"",
                   mName,
                   getClientSequence( client ),
                   client->receive.data );
#endif

         memcpy( client->receive.data + client->receive.used,
                 crlf,
                 sizeof crlf );
         client->receive.used += 2;    /* Don't add terminating NULL */

      }
   }
   else if (received == SOCKET_ERROR)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "%s: client %d recv() failed",
                  mName,
                  getClientSequence( client ));
      printWSerror("recv", wsErr);

      if (isFatalSocketError(wsErr))
      {
         shutdown(client->handle, 2); /* Fail both reads and writes  */
         return 0;                    /* Force termination of client */
      }
   }
   else {
      incrementClientBytesRead( client, received );
      client->receive.used += received;
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
closeSocket( SOCKET handle )
{
   static const char mName[] = "closeSocket";

   if ( handle == INVALID_SOCKET )
   {
      printmsg(0, "%s: Called for invalid socket", mName );
      panic();
   }

   closesocket( handle );

} /* closeSocket */

/*--------------------------------------------------------------------*/
/*       S M T P B u r p B u f f e r                                  */
/*                                                                    */
/*       Discard already processed data from the input buffer         */
/*--------------------------------------------------------------------*/

static void
SMTPBurpBuffer( SMTPClient *client )
{

   static const char mName[] = "SMTPBurpBuffer";

/*--------------------------------------------------------------------*/
/*                 Verify the status of the input buffer              */
/*--------------------------------------------------------------------*/

   if ( client->receive.length < client->receive.used )
   {
         printmsg(0, "%s: Client has used more bytes (%d) "
                     "than buffer bytes allocated (%d)",
                    mName,
                    getClientSequence( client ),
                    client->receive.used,
                    client->receive.length );

      panic();
   }

   if ( client->receive.used < client->receive.parsed )
   {
         printmsg(0, "%s: Client has parsed more bytes (%d) "
                     "than bytes in use (%d)",
                    mName,
                    getClientSequence( client ),
                    client->receive.parsed,
                    client->receive.used );

      panic();
   }

/*--------------------------------------------------------------------*/
/*     Discard any data we have already processed from the client     */
/*--------------------------------------------------------------------*/

   if ( client->receive.parsed > 0 )
   {
      client->receive.used -= client->receive.parsed;

      if ( client->receive.used > 0 )
         memmove( client->receive.data,
                  client->receive.data + client->receive.parsed,
                  client->receive.used);
      client->receive.parsed = 0;

   } /* if ( client->receive.parsed > 0 ) */

} /* SMTPBurpBuffer */

/*--------------------------------------------------------------------*/
/*       s e l e c t R e a d y S o c k e t s                          */
/*                                                                    */
/*       Perform select to determine what sockets are ready,          */
/*       waiting if needed                                            */
/*--------------------------------------------------------------------*/

KWBoolean
selectReadySockets( SMTPClient *master )
{
   static const char mName[] = "flagReadySockets";
   SMTPClient *current = master;

   fd_set readfds;
   int nReady;
   int nSelected = 0;
   int nTotal = 0;
   int maxSocket = 0;
   struct timeval timeoutPeriod;

   timeoutPeriod.tv_sec = (master->next == NULL) ? 900 : 30;
   timeoutPeriod.tv_usec = 0;

   FD_ZERO(&readfds);

/*--------------------------------------------------------------------*/
/*       Loop through the list of valid sockets, adding each one      */
/*       to the list of sockets to check and determining the          */
/*       shortest timeout of the sockets.                             */
/*--------------------------------------------------------------------*/

   do {

      time_t timeout = getClientTimeout( current );

      if ( isClientValid( current ) && ! isClientIgnored( current ))
      {
            FD_SET((unsigned)getClientHandle( current ), &readfds);

            if ( (int) getClientHandle( current ) > maxSocket )
               maxSocket = getClientHandle( current );

            nSelected++;
      }

      if ( timeout < timeoutPeriod.tv_sec )
          timeoutPeriod.tv_sec = timeout;

      nTotal++;
      current = current->next;

   } while( current );

/*--------------------------------------------------------------------*/
/*       If no sockets are to be checked and no clients are           */
/*       already in a state to processed, report the oddity           */
/*       and continue after a short pause                             */
/*--------------------------------------------------------------------*/

   if ( ! maxSocket )
   {
      if ( timeoutPeriod.tv_sec > 0 )
      {
         printmsg(0, "%s: All sockets of %d ignored!",
                     mName,
                     nTotal );
         ssleep( 10 );
      }
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*             Perform actual selection and check for errors          */
/*--------------------------------------------------------------------*/

   nReady = select(maxSocket, &readfds, NULL, NULL, &timeoutPeriod);

   if (nReady == SOCKET_ERROR)
   {
      int wsErr = WSAGetLastError();

      printmsg(0, "%s: select() of %d (out of %d) sockets failed" ,
               mName,
               nSelected,
               nTotal );
      printWSerror("select", wsErr);
      panic();
   }

   printmsg( nReady ? 4 : 8, "%s: %d of %d (out of %d) sockets were ready.",
             mName,
             nReady,
             nSelected,
             nTotal );

/*--------------------------------------------------------------------*/
/*                   Update list of sockets to process                */
/*--------------------------------------------------------------------*/

   current = master;

   do {
      if ( isClientValid(current) &&
           FD_ISSET((unsigned) getClientHandle( current ), &readfds ))
      {
         setClientReady( current, KWTrue );
         setClientProcess( current, KWTrue );
      }

      current = current->next;

   } while( current );

   return KWTrue;

} /* flagReadySockets */
