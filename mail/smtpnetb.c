/*--------------------------------------------------------------------*/
/*       s m t p n e t w . c                                          */
/*                                                                    */
/*       TCP/IP generic support for UUPC/extended SMTP                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2001 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include "smtpclnt.h"
#include "smtpnett.h"
#include <io.h>

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpnetb.c 1.4 2000/05/25 03:41:49 ahd v1-13g $
 *
 *    $Log: smtpnetb.c $
 *    Revision 1.4  2000/05/25 03:41:49  ahd
 *    Use more conservative buffering to avoid aborts
 *
 *    Revision 1.3  2000/05/12 12:35:45  ahd
 *    Annual copyright update
 *
 *    Revision 1.2  1999/02/21 04:09:32  ahd
 *    Support for BSMTP support, with routines for batch file I/O
 *    and breakout of TCP/IP routines into their own file.
 *
 */

/*--------------------------------------------------------------------*/
/*                      Global defines/variables                      */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtpnetb.c 1.4 2000/05/25 03:41:49 ahd v1-13g $");

/*--------------------------------------------------------------------*/
/*       g e t M o d e T i m e o u t                                  */
/*                                                                    */
/*       Determine timeout for specified client mode                  */
/*--------------------------------------------------------------------*/

time_t
getModeTimeout(SMTPMode mode)
{
   return 0;
} /* getModeTimeout */

/*--------------------------------------------------------------------*/
/*    I n i t i a l i z e N e t w o r k                               */
/*                                                                    */
/*    Start the Windows sockets DLL                                   */
/*--------------------------------------------------------------------*/

KWBoolean
InitializeNetwork (void)
{
   return KWTrue;

} /* InitializeNetwork */

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

   printmsg(0,"%s: Not supported in BSMTP", mName);
   panic();

   return INVALID_SOCKET;         /* Return failure to caller      */

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

   printmsg(0,"%s: Not supported in BSMTP", mName);
   panic();

   return INVALID_SOCKET;         /* Return failure to caller      */

} /* openSlave */

/*--------------------------------------------------------------------*/
/*       S M T P W r i t e                                            */
/*                                                                    */
/*       Pretend to write to open sock                                */
/*--------------------------------------------------------------------*/

size_t
SMTPWrite(SMTPClient *client,
          const char UUFAR *data,
          unsigned int len)
{

   printmsg(5,"***> %.125s", data);

/*--------------------------------------------------------------------*/
/*              Return byte count transmitted to caller               */
/*--------------------------------------------------------------------*/

   return len;

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
      return client->receive.used;

/*--------------------------------------------------------------------*/
/*                 Make sure our buffer is big enough                 */
/*--------------------------------------------------------------------*/

   if (client->receive.used >= client->receive.NetworkAllocated)
   {
      if (client->receive.NetworkAllocated < MAX_BUFFER_SIZE)
      {
         printmsg(2, "%s: Client %d buffer size doubled to %d bytes",
                    mName,
                    getClientSequence(client),
                    client->receive.NetworkAllocated);
         client->receive.NetworkAllocated *= 2;
         client->receive.NetworkBuffer =
                       realloc(client->receive.NetworkBuffer,
                                 client->receive.NetworkAllocated);
         checkref(client->receive.NetworkBuffer);

      } /* if (client->receive.NetworkAllocated < MAX_BUFFER_SIZE) */
      else {
          printmsg(0, "%s: Client %d overran of input buffer %d,"
                      " truncated.",
                      mName,
                      getClientSequence(client),
                      client->receive.NetworkAllocated);
          return client->receive.used;

      } /* else */

   } /* if (client->receive.used >= client->receive.NetworkAllocated) */

/*--------------------------------------------------------------------*/
/*                  Actually get our next data read                   */
/*--------------------------------------------------------------------*/

   received = recv(getClientHandle(client),
                   client->receive.NetworkBuffer + client->receive.used,
                   (int) (client->receive.NetworkAllocated - client->receive.used),
                   0);

   if (received == 0)
   {
      client->endOfTransmission = KWTrue;
      printmsg(0, "%s: client %d EOF on recv(%ld,%p,%d,%d)",
                  mName,
                  getClientSequence(client),
                  (long) getClientHandle(client),
                  client->receive.NetworkBuffer + client->receive.used,
                  (int) (client->receive.NetworkAllocated - client->receive.used),
                  0);
   }
   else if (received == -1)
   {
      printerr("stdin");
      return 0;

   }
   else {
      incrementClientBytesRead(client, (size_t) received);
      client->receive.used += (size_t) received;

      if (client->receive.next == NULL)
         client->receive.next = client->receive.NetworkBuffer;
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

   close(handle);

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

   do {
      if (isClientValid(current))
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

   printmsg(0,"%s: Not supported in BSMTP", mName);
   panic();

   return KWFalse;                  /* Report failure to caller   */
} /* isSocketReady */

/*--------------------------------------------------------------------*/
/*       g e t D e f a u l t H a n d l e                              */
/*                                                                    */
/*       Report default handle to initialize program to               */
/*--------------------------------------------------------------------*/

SOCKET
getDefaultHandle( void )
{
   return 0;                        /* Standard input                */
} /* getDefaultHandle */

/*--------------------------------------------------------------------*/
/*       g e t H o s t N a m e F r o m S o c k e t                    */
/*                                                                    */
/*       Report fixed dummy host name for processing                  */
/*--------------------------------------------------------------------*/

KWBoolean
getHostNameFromSocket( SMTPConnection *connection )
{
   strcpy(connection->hostAddr, "0.0.0.0");
   strcpy(connection->hostName, "BSMTP.host");
   connection->reverseLookup = KWTrue;

   return KWTrue;

} /* getHostNameFromSocket */
