/*--------------------------------------------------------------------*/
/*       s m t p c l n t . c                                          */
/*                                                                    */
/*       SMTP server support routines for clients                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2000 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: smtpclnt.c 1.22 2000/05/12 12:35:45 ahd Exp ahd $
 *
 *       Revision History:
 *       $Log: smtpclnt.c $
 *       Revision 1.22  2000/05/12 12:35:45  ahd
 *       Annual copyright update
 *
 *       Revision 1.21  1999/02/21 04:09:32  ahd
 *       Support for BSMTP support, with routines for batch file I/O
 *       and breakout of TCP/IP routines into their own file.
 *
 *       Revision 1.20  1999/01/17 17:19:16  ahd
 *       Give priority to accepting new connections
 *       Make initialization of slave and master connections more consistent
 *
 *       Revision 1.19  1999/01/08 02:21:05  ahd
 *       Convert currentfile() to RCSID()
 *
 *       Revision 1.18  1999/01/04 03:54:27  ahd
 *       Annual copyright change
 *
 *       Revision 1.17  1998/11/04 01:59:55  ahd
 *       Prevent buffer overflows when processing UIDL lines
 *       in POP3 mail.
 *       Add internal sanity checks for various client structures
 *       Convert various files to CR/LF from LF terminated lines
 *
 * Revision 1.16  1998/04/24  03:30:13  ahd
 * Use local buffers, not client->transmit.buffer, for output
 * Rename receive buffer, use pointer into buffer rather than
 *      moving buffered data to front of buffer every line
 * Restructure main processing loop to give more priority
 *      to client processing data already buffered
 * Add flag bits to client structure
 * Add flag bits to verb tables
 *
 *       Revision 1.15  1998/04/22 01:19:54  ahd
 *       Performance improvements for SMTPD data mode
 *
 *       Revision 1.14  1998/04/08 11:35:35  ahd
 *       CHange error processing for bad sockets
 *
 *       Revision 1.13  1998/03/08 23:10:20  ahd
 *       Improve timeout processing, UUXQT support
 *
 *       Revision 1.12  1998/03/08 04:50:04  ahd
 *       Allow setting client timeout for specific client
 *
 *       Revision 1.11  1998/03/06 06:52:34  ahd
 *       Shorten timeout processing for  ignored clients
 *
 *       Revision 1.10  1998/03/01 19:40:21  ahd
 *       First compiling POP3 server which accepts user id/password
 *
 *       Revision 1.9  1998/03/01 01:32:44  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.8  1997/11/29 13:03:13  ahd
 *       Clean up single client (hot handle) mode for OS/2, including correct
 *       network initialization, use unique client id (pid), and invoke all
 *       routines needed in main client loop.
 *
 *       Revision 1.7  1997/11/28 23:11:38  ahd
 *       Additional SMTP auditing, normalize formatting, more OS/2 SMTP fixes
 *
 *       Revision 1.6  1997/11/28 04:52:10  ahd
 *       Initial UUSMTPD OS/2 support
 *
 *       Revision 1.5  1997/11/26 03:34:11  ahd
 *       Correct SMTP timeouts, break out protocol from rest of daemon
 *
 *       Revision 1.4  1997/11/25 05:05:06  ahd
 *       More robust SMTP daemon
 *
 *       Revision 1.3  1997/11/24 02:52:26  ahd
 *       First working SMTP daemon which delivers mail
 *
 *       Revision 1.2  1997/11/21 18:15:18  ahd
 *       Command processing stub SMTP daemon
 *
 *       Revision 1.1  1997/06/03 03:25:31  ahd
 *       Initial revision
 *
 */

#include "uupcmoah.h"
#include "smtpclnt.h"
#include "smtpverb.h"
#include "smtpnetw.h"
#include "smtpnett.h"
#include "smtpdns.h"
#include "ssleep.h"
#include "catcher.h"
#include "memstr.h"

#include <limits.h>
#include <ctype.h>
#include <process.h>

/*--------------------------------------------------------------------*/
/*                    Global defines and variables                    */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtpclnt.c 1.22 2000/05/12 12:35:45 ahd Exp ahd $");

static size_t clientSequence = 0;

/*--------------------------------------------------------------------*/
/*       i n i t i a l i z e C l i e n t                              */
/*                                                                    */
/*       Initializer client structure for processing                  */
/*--------------------------------------------------------------------*/

SMTPClient *
initializeClient(SOCKET handle)
{
   static const char mName[] = "initializeClient";
   SMTPClient *client = malloc(sizeof *client);

   checkref(client);
   memset(client, 0, sizeof *client);

   client->magic    = SMTPC_MAGIC;
   client->connectTime = client->lastTransactionTime = time(NULL);

   /* Only way we can be zero client is no master, so use pid then */
   if (clientSequence)
      client->sequence = ++clientSequence;
   else
      client->sequence = getpid();

   /* Process client immediately */
   setClientProcess(client, KWTrue);

   /* No data is read previous to first pass through verb processor */
   setClientFlag(client, SF_NO_READ);

   if (terminate_processing)
      setClientMode(client, SM_EXITING);
   else
      setClientMode(client, SM_CONNECTED);

   setClientHandle(client, handle);

   if (! getHostNameFromSocket(&client->connection))
   {
      printmsg(0,"%s: Client %d IP address retrieval failed",
                  mName,
                  getClientSequence(client));
      freeClient(client);
      return NULL;
   }

/*--------------------------------------------------------------------*/
/*                  Allocate remaining buffers we need                */
/*--------------------------------------------------------------------*/

   client->receive.NetworkAllocated = BUFSIZ;
   client->receive.NetworkBuffer   =
                      malloc((size_t) client->receive.NetworkAllocated);
   checkref(client->receive.NetworkBuffer);

#ifdef UDEBUG
   memset(client->receive.NetworkBuffer,
          0,
          client->receive.NetworkAllocated);
#endif

   printmsg(1, "%s: Client %d accepted from %s %s",
               mName,
               getClientSequence(client),
               client->connection.hostName,
               client->connection.hostAddr);

   return client;

} /* initializeClient */

/*--------------------------------------------------------------------*/
/*       i n i t i a l i z e M a s t e r                              */
/*                                                                    */
/*       Initial master client, which listens for                     */
/*       actual client connections                                    */
/*--------------------------------------------------------------------*/

SMTPClient *
initializeMaster(const char *portName, time_t exitTime)
{
   static const char mName[] = "initializeMaster";
   SMTPClient *master = malloc(sizeof *master);

   checkref(master);

   memset(master, 0, sizeof *master);
   master->sequence = ++clientSequence;
   master->magic    = SMTPC_MAGIC;

   setClientMode(master, SM_MASTER);
   setClientHandle(master, openMaster(portName));

   /* Master socket never attempts to read data */
   setClientFlag(master, SF_NO_READ);

   master->connectTime = master->lastTransactionTime = time(NULL);
   master->listening   = KWTrue;

   if (getClientHandle(master) == INVALID_SOCKET)
   {
      printmsg(0,"%s: Unable to open master port for listening.",
                 mName);
      free(master);
      return NULL;
   }

   if (exitTime)
      master->terminationTime = exitTime;
   else
      master->terminationTime = LONG_MAX;

   return master;

} /* initializeMaster */

/*--------------------------------------------------------------------*/
/*       s e t C l i e n t C l o s e d                                */
/*                                                                    */
/*       Close network connection for client, if open                 */
/*--------------------------------------------------------------------*/

void
setClientClosed(SMTPClient *client)
{
   static const char mName[] = "setClientClosed";

   assertSMTP(client);

   if (getClientHandle(client) != INVALID_SOCKET)
   {
      printmsg(2,"%s: Closing client %d on handle %d",
                  mName,
                  getClientSequence(client),
                  getClientHandle(client));

      closeSocket(getClientHandle(client));
      setClientHandle(client, INVALID_SOCKET);
      client->endOfTransmission = KWTrue;
   }
#ifdef UDEBUG
   else
      printmsg(2,"%s: Client %d already closed",
                  mName,
                  getClientSequence(client));
#endif

}  /* setClientClosed */

/*--------------------------------------------------------------------*/
/*       f r e e C l i e n t                                          */
/*                                                                    */
/*       Release the resources associated with a client               */
/*--------------------------------------------------------------------*/

void
freeClient(SMTPClient *client)
{
   static const char mName[] = "freeClient";
   time_t now;

   assertSMTP(client);
   time(&now);

   printmsg(1,"%s: Dropping client %d, "
              "age was %d seconds, "
              "processed %d transactions.",
               mName,
               getClientSequence(client),
               now - client->connectTime,
               getClientMajorTransaction( client ));

/*--------------------------------------------------------------------*/
/*                        Perform housekeeping                        */
/*--------------------------------------------------------------------*/

   setClientClosed(client);
   cleanupTransaction(client);

/*--------------------------------------------------------------------*/
/*          Drop the memory we have pointers to which we own          */
/*--------------------------------------------------------------------*/

   if (client->clientName)
   {
      free(client->clientName);
      client->clientName = NULL;
   }

   if (client->receive.NetworkBuffer)
   {
      free(client->receive.NetworkBuffer);
      client->receive.NetworkBuffer = NULL;
   }

   if (client->transmit.NetworkBuffer)
   {
      free(client->transmit.NetworkBuffer);
      client->transmit.NetworkBuffer = NULL;
   }

   if (client->receive.DataBuffer)
   {
      free(client->receive.DataBuffer);
      client->receive.DataBuffer = NULL;
   }

   if (client->transmit.DataBuffer)
   {
      free(client->transmit.DataBuffer);
      client->transmit.DataBuffer = NULL;
   }

/*--------------------------------------------------------------------*/
/*           Drop ourselves from the linked list of clients           */
/*--------------------------------------------------------------------*/

   if (client->previous)
   {

      assertSMTP(client->previous);
      client->previous->next = client->next;
   }

   if (client->next)
   {
      assertSMTP(client->next);
      client->next->previous = client->previous;
   }

/*--------------------------------------------------------------------*/
/*                     Now drop the client memory                     */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   memset(client, 0xFF, sizeof *client);     /* Invalidate the data  */
#endif
   free(client);

} /* freeClient */

/*--------------------------------------------------------------------*/
/*       p r o c e s s C l i e n t                                    */
/*                                                                    */
/*       Processes one line of input data for a client                */
/*--------------------------------------------------------------------*/

void
processClient(SMTPClient *client)
{
   static const char mName[] = "processClient";
   client->ignoreUntilTime = 0;     /* If we're called, this out of
                                       date ... short circuit checks */

   assertSMTP(client);

   if (isClientFlag(client, SF_NO_READ))
   {
      clearClientFlag(client, SF_NO_READ);

#ifdef UDEBUG
      printmsg(5,"%s: No read processing for client %d",
               mName,
               getClientSequence(client));
#endif

      SMTPInvokeCommand(client);       /* Based on state          */
   }
   else {
      if (SMTPGetLine(client))
         SMTPInvokeCommand(client);    /* Process offered cmd     */
   }

} /* processClient */

/*--------------------------------------------------------------------*/
/*       i s C l i e n t V a l i d                                    */
/*                                                                    */
/*       Report if a client is valid                                  */
/*--------------------------------------------------------------------*/

KWBoolean
isClientValid(const SMTPClient *client)
{
   static const char mName[] = "isClientValid";
   KWBoolean result;

   assertSMTP(client);

   if (client->mode == SM_DELETE_PENDING)
      result = KWFalse;
   else if (getClientHandle(client) == INVALID_SOCKET)
      result = KWFalse;
   else if (getClientSocketError(client))
      result = KWFalse;
   else
      result = KWTrue;

   return result;

} /* isClientValid */

KWBoolean isClientEOF(const SMTPClient *client)
{
   return client->endOfTransmission;
}

/*--------------------------------------------------------------------*/
/*       i s C l i e n t T i m e d O u t                              */
/*                                                                    */
/*       Report if client was idle too long and should be             */
/*       terminated                                                   */
/*--------------------------------------------------------------------*/

KWBoolean
isClientTimedOut(const SMTPClient *client)
{
   static const char mName[] = "isClientTimedOut";
   time_t now;

   assertSMTP(client);

   /* Special case, since timeout is zero to force processing */
   if (getClientProcess(client))
      return KWFalse;

   /* Dead clients don't wear plaid ... or timeout */
   if (! isClientValid(client))
      return KWFalse;

   time(&now);

   /* If past absolute termination time, client is timed out */
   if (client->terminationTime > 0 &&
       (client->terminationTime <= now))
      return KWTrue;

   /* Handle case of very large time out */
   if (getClientTimeout(client) > now)
      return KWFalse;

   /* Not fair to timeout ignored client */
   if (isClientIgnored(client))
      return KWFalse;

   if ((getClientTimeout(client) +
       max(client->lastTransactionTime, client->ignoreUntilTime)) < now)
   {
#ifdef UDEBUG
      printmsg(2, "%s: Client %d last transaction time was %.24s "
                  "(time out after %d seconds)",
                  mName,
                  getClientSequence(client),
                  ctime(&client->lastTransactionTime),
                  getClientTimeout(client));
#endif
      return KWTrue;
   }
   else
      return KWFalse;

} /* isClientTimedOut */

/*--------------------------------------------------------------------*/
/*       i s C l i e n t I g n o r e d                                */
/*                                                                    */
/*       Report if client should not be processed at this time        */
/*--------------------------------------------------------------------*/

KWBoolean
isClientIgnored(const SMTPClient *client)
{
   static const char mName[] = "isClientIgnored";
   time_t now;

   assertSMTP(client);

   if (client->ignoreUntilTime == 0)
      return KWFalse;

   time(& now);

   if (client->ignoreUntilTime <= now)
   {
      printmsg(5, "%s: Client %d is active, time is valid.",
                  mName,
                  getClientSequence(client));
      return KWFalse;
   }
   else {
      printmsg(8,"%s: Client %d with handle %d ignored, %ld > %ld",
                  mName,
                  getClientSequence(client),
                  getClientHandle(client),
                  client->ignoreUntilTime,
                  now);
      return KWTrue;
   }

} /* isClientIgnored */

/*--------------------------------------------------------------------*/
/*       s e t C l i e n t M o d e                                    */
/*                                                                    */
/*       Set client into a new state                                  */
/*--------------------------------------------------------------------*/

void
setClientMode(SMTPClient *client, SMTPMode mode)
{
#ifdef UDEBUG
   static const char mName[] = "setClientMode";

   printmsg(mode == client->mode ? 8 : 3,
           "%s: Changing client %d from mode 0x%04x to mode 0x%04x",
            mName,
            getClientSequence(client),
            client->mode,
            mode);
#endif

   client->mode = mode;
}

/*--------------------------------------------------------------------*/
/*       g e t C l i e n t M o d e                                    */
/*                                                                    */
/*       Query the current client mode                                */
/*--------------------------------------------------------------------*/

SMTPMode
getClientMode(const SMTPClient *client)
{
   return client->mode;
}

/*--------------------------------------------------------------------*/
/*       i n c r e m e n t C l i e n t M a j o r                      */
/*       T r a n s a c t i o n                                        */
/*                                                                    */
/*       Count major transactions (mail delivers, etc) for client     */
/*--------------------------------------------------------------------*/

void
incrementClientMajorTransaction(SMTPClient *client)
{
   client->majorTransactions++;
}

/*--------------------------------------------------------------------*/
/*       g e t C l i e n t M a j o r T r a n s a c t i o n            */
/*                                                                    */
/*       Report major transactions for client                         */
/*--------------------------------------------------------------------*/

size_t
getClientMajorTransaction(SMTPClient *client)
{
   return client->majorTransactions;
}

/*--------------------------------------------------------------------*/
/*       g e t C l i e n t T e r m i n a t i o n T i m e              */
/*                                                                    */
/*       Report when this client must terminate processing            */
/*--------------------------------------------------------------------*/

time_t
getClientTerminationTime(const SMTPClient *client)
{
      return client->terminationTime;
} /* getClientTerminationTime */

void
setClientReady(SMTPClient *client, KWBoolean ready)
{
   client->ready = ready;
}

KWBoolean
getClientReady(const SMTPClient *client)
{
   return client->ready;
} /* getClientReady */

void setClientQueueRun(SMTPClient *client, KWBoolean needQueueRun)
{
   client->needQueueRun = needQueueRun;
}

KWBoolean
getClientQueueRun(const SMTPClient *client)
{
   return client->needQueueRun;
}

void
setClientProcess(SMTPClient *client, KWBoolean process)
{
   client->process = process;
}

KWBoolean
getClientProcess(const SMTPClient *client)
{
   return client->process;
} /* getClientProcess */

KWBoolean
getClientBufferedData(const SMTPClient *client)
{
   if (client->receive.NetworkUsed != 0)
   {

      if (memstr(client->receive.NetworkBuffer,
                 "\r\n",
                 client->receive.NetworkUsed) != NULL)
         return KWTrue;
      else
         return KWFalse;
   }
   else
      return KWFalse;

} /* getClientBufferedData */

SOCKET
getClientHandle(const SMTPClient *client)
{
   return client->connection.handle;
}

void
setClientHandle(SMTPClient *client, SOCKET handle)
{
   client->connection.handle = handle;
}

int
getClientSocketError(const SMTPClient *client)
{
   return client->connection.error;
}

void
setClientSocketError(SMTPClient *client, int error )
{
   client->connection.error = error;
}

time_t
getClientTimeout(const SMTPClient *client)
{
   time_t now;
   time_t maximumTimeout = LONG_MAX;

   assertSMTP(client);

   /* Clients ready to process should not be held up */
   if (getClientProcess(client))
      return 0;

   if ((client->ignoreUntilTime != 0) ||(client->terminationTime != 0))
      time(&now);

   /* Ignored clients timeout when they get out of penalty box */
   if (client->ignoreUntilTime != 0)
   {
      if (client->ignoreUntilTime > now)
         return client->ignoreUntilTime - now;
   }

   if (client->terminationTime > 0)
   {
      if ( client->terminationTime <= now )
         return 0;
      else
         maximumTimeout = client->terminationTime - now;

   }

   /* Use client specfied timeout, if provided */
   if ( client->timeout > 0 )
      return min( client->timeout, maximumTimeout );

   /* All other sockets timeout according to current client mode */
   return min( getModeTimeout(client->mode), maximumTimeout );

} /* getClientTimeout */

void
setClientTimeout( SMTPClient *client, time_t timeout )
{
   client->timeout = timeout;
} /* setClientTimeout */

void
setClientIgnore(SMTPClient *client, time_t delay)
{
   static const char mName[] = "setClientIgnore";

   time_t timerPop;

/*--------------------------------------------------------------------*/
/*               Handle special case of resetting timer               */
/*--------------------------------------------------------------------*/

   if (delay == 0)
   {
      client->ignoreUntilTime = 0;
      return;
   }

   time(&timerPop);

   timerPop += delay;

   if (client->ignoreUntilTime < timerPop)
      client->ignoreUntilTime = timerPop;
   else
      client->ignoreUntilTime += delay;

   printmsg(2,"%s: Ignoring client %d in mode 0x%04x "
               "for %d seconds (until %.24s)",
               mName,
               getClientSequence(client),
               getClientMode(client),
               delay,
               ctime(&client->ignoreUntilTime));

} /* getClientTimeout */

size_t
getClientSequence(const SMTPClient *client)
{

   assertSMTP(client);
   return client->sequence;
}

void incrementClientLinesWritten(SMTPClient *client)
{

   assertSMTP(client);
   client->transmit.bytesTransferred++;
}

size_t getClientLinesWritten(SMTPClient *client)
{

   assertSMTP(client);
   return client->transmit.linesTransferred;
}

void incrementClientBytesWritten(SMTPClient *client,
                               size_t increment)
{

   assertSMTP(client);
   client->transmit.bytesTransferred += increment;
}

size_t getClientBytesWritten(SMTPClient *client)
{

   assertSMTP(client);
   return client->transmit.bytesTransferred;
}

void incrementClientLinesRead(SMTPClient *client)
{

   assertSMTP(client);
   client->receive.linesTransferred++;
}

size_t getClientLinesRead(SMTPClient *client)
{

   assertSMTP(client);
   return client->receive.bytesTransferred;
}

void incrementClientBytesRead(SMTPClient *client,
                               size_t increment)
{

   assertSMTP(client);
   client->receive.bytesTransferred += increment;
}

size_t getClientBytesRead(SMTPClient *client)
{

   assertSMTP(client);
   return client->receive.bytesTransferred;
}

void incrementClientTrivialCount(SMTPClient *client)
{

   assertSMTP(client);
   client->trivialTransactions++;
}

size_t getClientTrivialCount(const SMTPClient *client)
{

   assertSMTP(client);
   return client->trivialTransactions;
}
