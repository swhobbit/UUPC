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
 *    $Id: smtpserv.c 1.1 1997/06/03 03:25:31 ahd Exp $
 *
 *    $Log: smtpserv.c $
 *    Revision 1.1  1997/06/03 03:25:31  ahd
 *    Initial revision
 *
 */

#include "uupcmoah.h"
#include "smtpserv.h"

#include "../uucico/uutcpip.h"
#include "ssleep.h"

RCSID("$Id: smtpclnt.c 1.1 1997/06/03 03:25:31 ahd Exp $");

currentfile();

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

   tm.tv_sec = 60;
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
#ifdef UDEBUG
      printmsg(5,"%s: Processing client %d, handle %d, mode 0x%04x",
                  mName,
                  getClientSequence( current),
                  getClientHandle( current),
                  getClientMode( current ));
#endif

      if ( isClientEOF( current ))
      {
         printmsg( 5, "%s: Client %d has reached EOF",
                      mName,
                      getClientSequence( current ));
         setClientReady( current, KWTrue );
         tm.tv_sec = 0;       /* Process client immediately */
      }
      else if ( isClientValid( current ))
      {
         if ( getClientHandle( current ) < 0 )
         {
            printmsg( 5, "%s: Client %d has invalid handle %d",
                         mName,
                         getClientSequence( current ),
                         getClientHandle( current ));
            panic();
         }

         if ( isClientIgnored( current ))
         {
            printmsg( 5, "%s: Client %d ignored",
                         mName,
                         getClientSequence( current ));

         } /* if ( isClientIgnored( current )) */
         else {

#ifdef UDEBUG
            printmsg( 5, "%s: Client %d valid",
                         mName,
                         getClientSequence( current ) );
#endif

            FD_SET((unsigned)getClientHandle( current ), &readfds);

            if ( (int) getClientHandle( current ) > maxSocket )
               maxSocket = getClientHandle( current );

            nSelected++;

         } /* else */

         if ( getClientTimeout( current ) < tm.tv_sec )
            tm.tv_sec = getClientTimeout( current );

      } /* if ( isClientValid( current )) */
      else {
         printmsg( 5, "%s: Client %d invalid",
                      mName,
                      getClientSequence( current ));
         setClientReady( current, KWTrue );
         tm.tv_sec = 0;       /* Process client immediately */
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
           FD_ISSET((unsigned) getClientHandle( current ), &readfds ))
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

   static const char mName[] = "dropTerminatedClients";
   int freed = 0;
   int total = 0;

   while( current->next != NULL )
   {
      SMTPClient *next = current->next;
      total++;

      if ( ! isClientValid( next ))
      {
         current->next = next->next;   /* Drop current from list */
         freeClient( next );
         freed++;
      }
      else
        current = next;
   }

   printmsg( (freed > 0) ? 1 : 5,
            "%s: freed %d of %d client connections.",
            mName,
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
         setClientMode(current, SM_EXITING);
         processClient( current );
         ssleep(1);
         processClient( current );
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

   freeClient( master );

} /* dropAllClients */
