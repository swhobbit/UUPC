/*--------------------------------------------------------------------*/
/*       s m t p c l n t . c                                          */
/*                                                                    */
/*       SMTP server support routines for clients                     */
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
 *       $Id: smtpclnt.c 1.2 1997/11/21 18:15:18 ahd Exp $
 *
 *       Revision History:
 *       $Log: smtpclnt.c $
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
#include "ssleep.h"

#include <ctype.h>

/*--------------------------------------------------------------------*/
/*                    Global defines and variables                    */
/*--------------------------------------------------------------------*/

currentfile();

RCSID("$Id: smtpclnt.c 1.2 1997/11/21 18:15:18 ahd Exp $");

static long clientSequence = 0;

/*--------------------------------------------------------------------*/
/*       i n i t i a l i z e C l i e n t                              */
/*                                                                    */
/*       Initializer client structure for processing                  */
/*--------------------------------------------------------------------*/

SMTPClient *
initializeClient( SOCKET handle, KWBoolean master )
{
   SMTPClient *client = malloc( sizeof *client );

   checkref( client );
   memset( client, 0, sizeof *client );

   client->sequence = ++clientSequence;
   setClientMode( client, SM_CONNECTED );
   setClientProcess( client, KWTrue );

/*--------------------------------------------------------------------*/
/*       Either accept a client from the master socket, or (if not    */
/*       master socket passed in), use the socket passed in as our    */
/*       actual connection.                                           */
/*--------------------------------------------------------------------*/

   if ( master )
   {
      setClientHandle( client, openSlave( handle ));

      if ( getClientHandle( client ) == INVALID_SOCKET )
      {
         freeClient( client );
         return NULL;
      }
   }
   else
      setClientHandle( client, handle );

/*--------------------------------------------------------------------*/
/*                  Allocate remaining buffers we need                */
/*--------------------------------------------------------------------*/

   client->receive.length = BUFSIZ;
   client->receive.data   = malloc( client->receive.length );
   checkref( client->receive.data );

   client->transmit.length = BUFSIZ;
   client->transmit.data = malloc( client->transmit.length );
   checkref( client->transmit.data );

#ifdef UDEBUG
   memset( client->transmit.data, 0, client->transmit.length );
   memset( client->receive.data, 0, client->receive.length );
#endif

   client->connectTime = client->lastTransactionTime = time( NULL );

   return client;

} /* initializeClient */

/*--------------------------------------------------------------------*/
/*       i n i t i a l i z e M a s t e r                              */
/*                                                                    */
/*       Initial master client, which listens for                     */
/*       actual client connections                                    */
/*--------------------------------------------------------------------*/

SMTPClient *
initializeMaster( const char *portName, time_t exitTime )
{
   static const char mName[] = "initializeMaster";
   SMTPClient *master = malloc( sizeof *master);

   checkref( master );
   memset( master, 0, sizeof *master);
   master->sequence = ++clientSequence;
   setClientMode(master, SM_MASTER);

   setClientHandle( master, openMaster( portName ) );

   if ( getClientHandle( master ) == INVALID_SOCKET )
   {
      printmsg(0,"%s: Unable to open master port for listening.",
                 mName );
      free( master );
      return NULL;
   }

   master->connectTime = master->lastTransactionTime = time( NULL );

   return master;

} /* initializeMaster */

/*--------------------------------------------------------------------*/
/*       s e t C l i e n t C l o s e d                                */
/*                                                                    */
/*       Close network connection for client, if open                 */
/*--------------------------------------------------------------------*/

void
setClientClosed( SMTPClient *client )
{
   static const char mName[] = "setClientClosed";

   if ( client->handle != INVALID_SOCKET )
   {
      printmsg(2,"%s: Closing client %d on handle %d",
                  mName,
                  getClientSequence( client),
                  getClientHandle( client) );

      closeSocket( getClientHandle( client ));
      client->handle = INVALID_SOCKET;
      client->endOfTransmission = KWTrue;
   }
#ifdef UDEBUG
   else
      printmsg(2,"%s: Client %d already closed",
                  mName,
                  getClientSequence( client) );
#endif

}  /* setClientClosed */

/*--------------------------------------------------------------------*/
/*       f r e e C l i e n t                                          */
/*                                                                    */
/*       Release the resources associated with a client               */
/*--------------------------------------------------------------------*/

void
freeClient( SMTPClient *client )
{
   static const char mName[] = "freeClient";

   printmsg(2,"%s: Freeing client %d",
            mName,
            getClientSequence( client ));

   setClientClosed( client );
   cleanupClientMail( client );

   if ( client->SMTPName )
   {
      free( client->SMTPName );
      client->SMTPName = NULL;
   }

   if ( client->receive.data )
   {
      free( client->receive.data );
      client->receive.data = NULL;
   }

   if ( client->transmit.data )
   {
      free( client->transmit.data );
      client->transmit.data = NULL;
   }

   free( client );

} /* freeClient */

/*--------------------------------------------------------------------*/
/*       p r o c e s s C l i e n t                                    */
/*                                                                    */
/*       Processes one line of input data for a client                */
/*--------------------------------------------------------------------*/

void
processClient( SMTPClient *client )
{
   static const char mName[] = "processClient";
   client->ignoreUntilTime = 0;     /* If we're called, this out of
                                       date ... short circuit checks */

   switch( getClientMode( client ) )
   {

      /* First command doesn't read any data before response */
      case SM_CONNECTED:
         if ( getClientLinesWritten( client ) > 0 )
         {
            printmsg(0, "%s: Client %d "
                        "returned to initial command state "
                        "after first message, terminating client.",
                        mName,
                        getClientSequence( client ));
            setClientMode(client, SM_ABORT);
            break;
         }
         /* Fall through */

      /* Master socket also never reads data */
      case SM_MASTER:
         /* Fall through */

      /* Cleanup commands have no data left to read */
      case SM_ABORT:
      case SM_EXITING:
      case SM_TIMEOUT:
         SMTPInvokeCommand( client );  /* Process command by state   */
         break;

      default:
         if ( SMTPGetLine( client ) )
               SMTPInvokeCommand( client );  /* Process offered cmd  */
         break;
   }

   /* Flag this client was active */
   time( &client->lastTransactionTime );

} /* processClient */

/*--------------------------------------------------------------------*/
/*       i s C l i e n t V a l i d                                    */
/*                                                                    */
/*       Report if a client is valid                                  */
/*--------------------------------------------------------------------*/

KWBoolean
isClientValid( const SMTPClient *client )
{
   static const char mName[] = "isClientValid";
   KWBoolean result;

   if (client->mode == SM_INVALID )
      result = KWFalse;
   else
      result = KWTrue;

#ifdef UDEBUG
   printmsg( result ? 10: 5, "%s: Client %d is %svalid.",
               mName,
               getClientSequence( client ),
               result ? "quite " : "in" );
#endif

   return result;

} /* isClientValid */

KWBoolean isClientEOF( const SMTPClient *client )
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
isClientTimedOut( const SMTPClient *client )
{
   time_t now;

   time(&now);

   /* Handle case of very large time out */
   if (getClientTimeout(client) > now)
      return KWFalse;

   if ( (getClientTimeout(client) + client->lastTransactionTime) < now)
   {
#ifdef UDEBUG
      printmsg(2, "%s: Client %d last transaction time was %.24s "
                  "(time out after %d seconds)",
                  ctime( &client->lastTransactionTime ),
                  getClientTimeout(client) );
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
isClientIgnored( const SMTPClient *client )
{
   static const char mName[] = "isClientIgnored";
   time_t now;

   if ( client->ignoreUntilTime == 0 )
      return KWFalse;

   time( & now );

   if ( client->ignoreUntilTime <= now )
   {
      printmsg( 5, "%s: Client %d is active, time is valid.",
                  mName,
                  getClientSequence( client ) );
      return KWFalse;
   }
   else {
      printmsg(4,"%s: Client %d with handle %d ignored, %ld > %ld",
                  mName,
                  getClientSequence( client ),
                  getClientHandle( client ),
                  client->ignoreUntilTime,
                  now);
      return KWTrue;
   }

   printmsg(0,"%s: Reached impossible point for client %d." ,
               mName,
               getClientSequence( client ) );
   panic();

} /* isClientIgnored */

/*--------------------------------------------------------------------*/
/*       s e t C l i e n t M o d e                                    */
/*                                                                    */
/*       Set client into a new state                                  */
/*--------------------------------------------------------------------*/

void
setClientMode(SMTPClient *client, SMTPMode mode )
{
#ifdef UDEBUG
   static const char mName[] = "setClientMode";

   printmsg(2,
           "%s: Changing client %d from mode 0x%04x to mode 0x%04x",
            mName,
            getClientSequence( client ),
            client->mode,
            mode );
#endif

   client->mode = mode;
}

SMTPMode
getClientMode( const SMTPClient *client )
{
   return client->mode;
}

void
setClientReady(SMTPClient *client, KWBoolean ready )
{
   client->ready = ready;
}

KWBoolean
getClientReady( const SMTPClient *client )
{
   return client->ready;
} /* getClientReady */

void
setClientProcess(SMTPClient *client, KWBoolean process )
{
   client->process = process;
}

KWBoolean
getClientProcess( const SMTPClient *client )
{
   return client->process;
} /* getClientProcess */

KWBoolean
getClientBufferedData( const SMTPClient *client )
{
   if (client->receive.parsed < client->receive.used )
      return KWTrue;
   else
      return KWFalse;

} /* getClientBufferedData */

SOCKET
getClientHandle( const SMTPClient *client )
{
   return client->handle;
}

void
setClientHandle( SMTPClient *client, SOCKET handle )
{
   client->handle = handle;
}

time_t
getClientTimeout( const SMTPClient *client )
{

   if ( client->ignoreUntilTime != 0 )
   {
      time_t now;
      time( &now );

      if ( client->ignoreUntilTime > now )
         return client->ignoreUntilTime - now +
                getModeTimeout( client->mode );
   }

   return getModeTimeout( client->mode );

} /* getClientTimeout */

void
setClientIgnore( SMTPClient *client, time_t delay )
{

   time_t timerPop;

/*--------------------------------------------------------------------*/
/*               Handle special case of resetting timer               */
/*--------------------------------------------------------------------*/

   if ( delay == 0 )
   {
      client->ignoreUntilTime = 0;
      return;
   }

   time( &timerPop );

   timerPop += delay;

   if ( client->ignoreUntilTime > timerPop )
      client->ignoreUntilTime = timerPop;
   else
      client->ignoreUntilTime += delay;

} /* getClientTimeout */

int
getClientSequence( const SMTPClient *client )
{
   return client->sequence;
}

void incrementClientLinesWritten( SMTPClient *client )
{
   client->transmit.bytesTransferred++;
}

size_t getClientLinesWritten( SMTPClient *client )
{
   return client->transmit.linesTransferred;
}

void incrementClientBytesWritten( SMTPClient *client,
                               size_t increment)
{
   client->transmit.bytesTransferred += increment;
}

size_t getClientBytesWritten( SMTPClient *client )
{
   return client->transmit.bytesTransferred;
}

void incrementClientLinesRead( SMTPClient *client )
{
   client->receive.linesTransferred++;
}

size_t getClientLinesRead( SMTPClient *client )
{
   return client->receive.bytesTransferred;
}

void incrementClientBytesRead( SMTPClient *client,
                               size_t increment)
{
   client->receive.bytesTransferred += increment;
}

size_t getClientBytesRead( SMTPClient *client )
{
   return client->receive.bytesTransferred;
}

/*--------------------------------------------------------------------*/
/*       c l e a n u p C l i e n t M a i l                            */
/*                                                                    */
/*       reset variables for a mail command                           */
/*--------------------------------------------------------------------*/

void
cleanupClientMail( SMTPClient *client )
{
   if ( client->imf )
   {
      imclose( client->imf );
      client->imf = NULL;
   }

   if ( client->addressCount )
   {
      size_t count;
      for ( count = 0; count < client->addressCount; count ++ )
      {
         free( client->address[ count ] );
         client->address[ count ] = NULL;
      }

      client->addressCount = 0;
   }

   if ( client->address )
   {
      free( client->address );
      client->address = NULL;
   }

   if ( client->sender )
   {
      free( client->sender );
      client->sender = NULL;
   }

} /* cleanupClientMail */
