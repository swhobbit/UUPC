/*--------------------------------------------------------------------*/
/*       s m t p c l n t . c                                          */
/*                                                                    */
/*       SMTP server support routines for clients                     */
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
 *       $Id: smtpclnt.c 1.11 1998/03/06 06:52:34 ahd Exp $
 *
 *       Revision History:
 *       $Log: smtpclnt.c $
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
#include "smtpdns.h"
#include "ssleep.h"
#include "catcher.h"

#include <limits.h>
#include <ctype.h>
#include <process.h>

/*--------------------------------------------------------------------*/
/*                    Global defines and variables                    */
/*--------------------------------------------------------------------*/

currentfile();

RCSID("$Id: smtpclnt.c 1.11 1998/03/06 06:52:34 ahd Exp $");

static size_t clientSequence = 0;

/*--------------------------------------------------------------------*/
/*       i n i t i a l i z e C l i e n t                              */
/*                                                                    */
/*       Initializer client structure for processing                  */
/*--------------------------------------------------------------------*/

SMTPClient *
initializeClient(SOCKET handle, KWBoolean master)
{
   static const char mName[] = "initializeClient";
   SMTPClient *client = malloc(sizeof *client);

   checkref(client);
   memset(client, 0, sizeof *client);

   client->sequence = ++clientSequence;
   setClientProcess(client, KWTrue);
   client->connectTime = client->lastTransactionTime = time(NULL);

   if (terminate_processing)
      setClientMode(client, SM_EXITING);
   else
      setClientMode(client, SM_CONNECTED);

/*--------------------------------------------------------------------*/
/*       Either accept a client from the master socket, or (if not    */
/*       master socket passed in), use the socket passed in as our    */
/*       actual connection.                                           */
/*--------------------------------------------------------------------*/

   if (master)
   {
      setClientHandle(client, openSlave(handle));

      if (getClientHandle(client) == INVALID_SOCKET)
      {
         freeClient(client);
         return NULL;
      }
   }
   else {
      InitWinsock();
      client->sequence = getpid();
      setClientHandle(client, handle);
   }

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

   client->receive.length = 10240;
   client->receive.data   = malloc((size_t) client->receive.length);
   checkref(client->receive.data);

   client->transmit.length = BUFSIZ;
   client->transmit.data = malloc(client->transmit.length);
   checkref(client->transmit.data);

#ifdef UDEBUG
   memset(client->transmit.data, 0, client->transmit.length);
   memset(client->receive.data, 0, client->receive.length);
#endif

   printmsg(1, "%s: Client %d accepted from %s",
               mName,
               getClientSequence(client),
               client->connection.hostName);

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

   setClientMode(master, SM_MASTER);
   setClientHandle(master, openMaster(portName));
   master->connectTime = master->lastTransactionTime = time(NULL);

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

   if (client->receive.data)
   {
      free(client->receive.data);
      client->receive.data = NULL;
   }

   if (client->transmit.data)
   {
      free(client->transmit.data);
      client->transmit.data = NULL;
   }

/*--------------------------------------------------------------------*/
/*           Drop ourselves from the linked list of clients           */
/*--------------------------------------------------------------------*/

   if (client->previous)
      client->previous->next = client->next;

   if (client->next)
      client->next->previous = client->previous;

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

   switch(getClientMode(client))
   {

      /* First command doesn't read any data before response */
      case SM_CONNECTED:
         if (getClientLinesWritten(client) > 0)
         {
            printmsg(0, "%s: Client %d "
                        "returned to initial command state "
                        "after first message, terminating client.",
                        mName,
                        getClientSequence(client));
            setClientMode(client, SM_ABORT);
            break;
         }
         /* Fall through */

      /* Loading data (used by POP only) also never reads data */
      case SM_LOAD_MBOX:
      case SM_SEND_DATA:
         /* Fall through */

      /* Master socket also never reads data */
      case SM_MASTER:
         /* Fall through */

      /* Cleanup commands have no data left to read */
      case SM_ABORT:
      case SM_EXITING:
      case SM_TIMEOUT:
         SMTPInvokeCommand(client);    /* Process command by state   */
         break;

      default:
         if (SMTPGetLine(client))
            SMTPInvokeCommand(client);    /* Process offered cmd     */
         break;
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

   if (client->mode == SM_DELETE_PENDING)
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
   if (client->receive.parsed < client->receive.used)
      return KWTrue;
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

time_t
getClientTimeout(const SMTPClient *client)
{
   /* Clients ready to process should not be held up */
   if (getClientProcess(client))
      return 0;

   /* Ignored clients timeout when they get out of penalty box */
   if (client->ignoreUntilTime != 0)
   {
      time_t now;
      time(&now);

      if (client->ignoreUntilTime > now)
         return client->ignoreUntilTime - now;
   }

   /* Use client specfied timeout, if provided */
   if ( client->timeout > 0 )
      return client->timeout;

   /* All other sockets timeout according to current client mode */
   return getModeTimeout(client->mode);

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
   return client->sequence;
}

void incrementClientLinesWritten(SMTPClient *client)
{
   client->transmit.bytesTransferred++;
}

size_t getClientLinesWritten(SMTPClient *client)
{
   return client->transmit.linesTransferred;
}

void incrementClientBytesWritten(SMTPClient *client,
                               size_t increment)
{
   client->transmit.bytesTransferred += increment;
}

size_t getClientBytesWritten(SMTPClient *client)
{
   return client->transmit.bytesTransferred;
}

void incrementClientLinesRead(SMTPClient *client)
{
   client->receive.linesTransferred++;
}

size_t getClientLinesRead(SMTPClient *client)
{
   return client->receive.bytesTransferred;
}

void incrementClientBytesRead(SMTPClient *client,
                               size_t increment)
{
   client->receive.bytesTransferred += increment;
}

size_t getClientBytesRead(SMTPClient *client)
{
   return client->receive.bytesTransferred;
}

void incrementClientTrivialCount(SMTPClient *client)
{
   client->trivialTransactions++;
}

size_t getClientTrivialCount(const SMTPClient *client)
{
   return client->trivialTransactions;
}
