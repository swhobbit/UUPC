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
 *    $Id: uusmtpd.c 1.1 1997/05/20 03:55:46 ahd v1-12s $
 *
 *    $Log$
 */

#include "uupcmoah.h"
#include "smtpclnt.h"
#include "smtpverb.h"
#include "ssleep.h"

currentfile();
RCSID("$Id$");

/*--------------------------------------------------------------------*/
/*       i n i t i a l i z e C l i e n t                              */
/*                                                                    */
/*       Initializer client structure for processing                  */
/*--------------------------------------------------------------------*/

SMTPClient *
initializeClient( RemoteConnection *connection )
{
   SMTPClient *client = malloc( sizeof *client );

   checkref( client );

   memset( client, 0, sizeof *client );
   client->connection    = connection;
   client->mode          = SM_CONNECTED;
   client->ready         = KWTrue;         /* We act first            */

   client->senderLength  = MAXADDR;
   client->sender        = malloc( client->senderLength );
   checkref( client->sender );

   client->receiveBufferLength = MAXPACK;
   client->receiveBuffer = malloc( client->receiveBufferLength );
   checkref( client->receiveBuffer );

   return client;

} /* initializeClient */

/*--------------------------------------------------------------------*/
/*       i s C l i e n t V a l i d                                    */
/*                                                                    */
/*       Report if a client is valid                                  */
/*--------------------------------------------------------------------*/

KWBoolean
isClientValid( const SMTPClient *client )
{
   if (client->mode == SM_INVALID )
      return KWFalse;
   else
      return KWTrue;

} /* isClientValid */

KWBoolean
isClientIgnored( const SMTPClient *client )
{
   time_t now;

   if ( client->ignoreUntilTime == 0 )
      return KWFalse;

   time( & now );

   if ( client->ignoreUntilTime <= now )
      return KWFalse;
   else
      return KWTrue;

} /* isClientIgnored */

KWBoolean
isClientReady( const SMTPClient *client )
{
   return client->ready;
} /* isClientReady */

/*--------------------------------------------------------------------*/
/*       f r e e C l i e n t                                          */
/*                                                                    */
/*       Release the resources associated with a client               */
/*--------------------------------------------------------------------*/

void
freeClient( SMTPClient *client )
{
   if ( client->connection )
   {
      freeConnection( client->connection );
      client->connection = NULL;
   }

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

   if ( client->receiveBuffer )
   {
      free( client->receiveBuffer );
      client->receiveBuffer = NULL;
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
   restoreConnection( client->connection );

   client->ignoreUntilTime = 0;     /* If we're called, this out of
                                       date ... short circuit checks */

   if ( SMTPRead( client,
                  client->receiveBuffer,
                  client->receiveBufferLength,
                  getModeTimeout( getClientMode( client ))))
   {
      SMTPResponse( client, -SR_PE_NOTIMPL,"Your command was:" );
      SMTPResponse( client, -SR_PE_NOTIMPL, client->receiveBuffer );
   }
   else
      setClientMode(client, SM_TERMINATED);

   SMTPResponse( client, SR_PE_NOTIMPL, "SMTP is not yet available.");

   saveConnection( client->connection );

} /* processClient */

void
setClientMode(SMTPClient *client, SMTPMode mode )
{
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

KWBoolean getClientReady( const SMTPClient *client )
{
   return client->ready;
} /* getClientReady */

int getClientHandle( SMTPClient *client )
{
   return client->connection->handle;
}

time_t
getClientTimeout( const SMTPClient *client )
{

   if ( client->ignoreUntilTime != 0 )
   {
      time_t now;
      time( &now );

      if ( client->ignoreUntilTime > now )
         return client->ignoreUntilTime - now;
   }

   return getModeTimeout( client->mode );

} /* getClientTimeout */
