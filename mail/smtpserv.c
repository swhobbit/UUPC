/*--------------------------------------------------------------------*/
/*       s m t p s e r v . c                                          */
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
#include "smtpserv.h"
#include "smtpverb.h"

#include "../uucico/uutcpip.h"
#include "ssleep.h"

RCSID("$Id");

currentfile();

/*--------------------------------------------------------------------*/
/*       i n i t i a l i z e M a s t e r                              */
/*                                                                    */
/*       Initial master client client, which listens for              */
/*       actual client connections                                    */
/*--------------------------------------------------------------------*/

SMTPClient *
initializeMaster( const char *portName, time_t exitTime )
{
   static const char mName[] = "initializeMaster";
   int port = 25;
   SMTPClient *master = malloc( sizeof *master);

   checkref( master );
   memset( master, 0, sizeof *master);

   if (( portName != NULL ) && isdigit( *portName ))
      port = atol( portName );

   if (passiveopenline( (char *) portName, port, KWFalse ))
   {
      printmsg(0,"%s: Unable to open master port for listening on port %d.",
                 (int) port );
      free( master );
      return NULL;
   }

   master->connection = malloc( sizeof *(master->connection) );
   checkref( master->connection );
   memset( master->connection, 0, sizeof *(master->connection));
   saveConnection( master->connection );

   setClientMode(master, SM_MASTER);

   return master;

} /* initializeMaster */

/*--------------------------------------------------------------------*/
/*       a c c e p t C l i e n t                                      */
/*                                                                    */
/*       Accept a new client connection on master socket              */
/*--------------------------------------------------------------------*/

RemoteConnection *
acceptClient( )
{
   static const char mName[] = "acceptClient";
   RemoteConnection *connection = NULL;

   printmsg(0,"%s: Accepting new client.", mName );

   if (!WaitForNetConnect( 0 ))
      return NULL;

   connection = malloc( sizeof *connection );
   checkref( connection );
   memset( connection, 0, sizeof *connection);
   saveConnection( connection );

   return connection;

} /* acceptClient */

/*--------------------------------------------------------------------*/
/*       a c c e p t H o t C l i e n t                                */
/*                                                                    */
/*       Accept a client which was opened externally to our           */
/*       processing                                                   */
/*--------------------------------------------------------------------*/

RemoteConnection *
acceptHotClient( int handle )
{
   static const char mName[] = "acceptClient";
   RemoteConnection *connection;

   printmsg(0,"%s: Accepting new hot client on handle %d.",
            mName,
            handle);

   SetComHandle( handle );

   connection = malloc( sizeof *connection );
   checkref( connection );
   memset( connection, 0, sizeof *connection);
   saveConnection( connection );

   return connection;

} /* acceptClient */

/*--------------------------------------------------------------------*/
/*       f l a g R e a d y S o c k e t s                              */
/*                                                                    */
/*       Perform select to determine what sockets are ready,          */
/*       waiting if needed                                            */
/*--------------------------------------------------------------------*/

KWBoolean
flagReadySockets( SMTPClient *master )
{
   static const char mName[] = "flagReadySockets";
   SMTPClient *current = master;

   fd_set readfds;
   int nReady;
   int nSelected = 0;
   int nTotal = 0;
   int maxSocket = 0;
   time_t now;
   struct timeval tm;

   tm.tv_sec = 3600;
   tm.tv_usec = 0;

   FD_ZERO(&readfds);

   time( &now );

/*--------------------------------------------------------------------*/
/*       Loop through the list of valid sockets, adding each one      */
/*       to the list of sockets to check and determining the          */
/*       shortest timeout of the sockets.                             */
/*                                                                    */
/*       This computation actually allows a connection which          */
/*       should timeout to have its life extended so long as other    */
/*       sockets are busy; this cheat is an explicit bias towards     */
/*       not dropping connections on a busy server.                   */
/*--------------------------------------------------------------------*/

   do {

      if ( isClientValid( current ))
      {

         if ( ! isClientIgnored( current ))
         {
            printmsg( 5, "%s: Valid client has handle %d and timeout %d",
                         mName,
                         getClientHandle( current ),
                         getClientTimeout( current ) );

            FD_SET(getClientHandle( current ), &readfds);

            if ( getClientHandle( current ) > maxSocket )
               maxSocket = getClientHandle( current );

            nSelected++;
         }

         if ( getClientTimeout( current ) < tm.tv_sec )
            tm.tv_sec = getClientTimeout( current );
      }

      nTotal++;
      current = current->next;

   } while( current );

   if ( ! maxSocket )
   {
      printmsg(0, "%s: All sockets of %d ignored!",
                  mName,
                  nTotal );
      ssleep( tm.tv_sec ? tm.tv_sec : 1);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*             Perform actual selection and check for errors          */
/*--------------------------------------------------------------------*/

   nReady = select(maxSocket, &readfds, NULL, NULL, &tm);

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
   else if (nReady == 0)
   {
      printmsg(5, "%s: select() timed out for %d (out of %d) sockets",
                  mName,
                  nSelected,
                  nTotal );
      return KWFalse;
   }

   printmsg( 4, "%s: %d of %d (out of %d) sockets were ready.",
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
           FD_ISSET(getClientHandle( current ), &readfds ))
      {
         setClientReady( current, KWTrue );
      }

      current = current->next;

   } while( current );

   return KWTrue;

} /* flagReadySockets */

/*--------------------------------------------------------------------*/
/*       p r o c e s s R e a d y S o c k e t s                        */
/*                                                                    */
/*       Process clients that have ready sockets                      */
/*--------------------------------------------------------------------*/

KWBoolean
processReadySockets( SMTPClient *current )
{
   while ( current != NULL )
   {
      if ( getClientReady( current ))
      {
         processClient( current );
         setClientReady( current, KWFalse );
      }

      current = current->next;
   }

   return KWTrue;

} /* processReadySockets */

/*--------------------------------------------------------------------*/
/*       a d d C l i e n t                                            */
/*                                                                    */
/*       Add a new client to the list of clients to be serviced       */
/*--------------------------------------------------------------------*/

void
addClient( SMTPClient *master, SMTPClient *client )
{
   client->next = master->next;
   master->next = client;
} /* addClient */

/*--------------------------------------------------------------------*/
/*       d r o p T e r m i n a t e d C l i e n t s                    */
/*                                                                    */
/*       Perform all clean-up processing for clients which            */
/*       are no longer valid                                          */
/*--------------------------------------------------------------------*/

void
dropTerminatedClients( SMTPClient *current )
{

   int closed = 0;
   int freed = 0;
   int total = 0;

   while( current->next != NULL )
   {
      SMTPClient *next = current->next;
      total++;

      if ( getClientMode( next ) == SM_TERMINATED )
      {
         restoreConnection( next->connection );
         closeline();
         setClientMode( next, SM_INVALID );
         current = next;
         closed++;
      }
      else if ( ! isClientValid( next ))
      {
         if ( getClientHandle( next ) != INVALID_SOCKET )
            closeline();

         current->next = next->next;   /* Drop current from list */
         freeClient( current );
         freed++;
      }
      else
        current = next;
   }

   printmsg( ((closed + freed) > 0) ? 1 : 5,
            "Closed %d and freed %d of %d client connections.",
            closed,
            freed,
            total );

} /* dropTerminatedClients */

/*--------------------------------------------------------------------*/
/*       d r o p A l l C l i e n t s                                  */
/*                                                                    */
/*       Drop all clients, including the master client, from the      */
/*       list of clients to process                                   */
/*--------------------------------------------------------------------*/

void
dropAllClients( SMTPClient *master )
{
   static const char mName[] = "dropAllClients";
   SMTPClient *current;
   int count = 0;

   printmsg(1,"%s: Dropping all clients prior to program termination.",
               mName );

   dropTerminatedClients( master );

   current = master->next;

   while( current != NULL )
   {

      if ( isClientValid( current ))
      {
         SMTPResponse( current,
                       SR_TE_SHUTDOWN,
                       "SMTP Server shutting down.");
         setClientMode(current, SM_TERMINATED);
         count ++;
      }
      current = current->next;
   }

   if ( count )
   {
      ssleep( 1 );                     /* Let messages go out        */
      dropTerminatedClients( master ); /* Terminate active clients   */
   }

   dropTerminatedClients( master ); /* Free all remaining clients    */

/*--------------------------------------------------------------------*/
/*                   Drop the master client itself                    */
/*--------------------------------------------------------------------*/

   terminateCommunications();
   freeClient( master );

} /* dropAllClients */
