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
 *    $Id: smtpserv.c 1.2 1997/11/21 18:15:18 ahd Exp $
 *
 *    $Log: smtpserv.c $
 *    Revision 1.2  1997/11/21 18:15:18  ahd
 *    Command processing stub SMTP daemon
 *
 *    Revision 1.1  1997/06/03 03:25:31  ahd
 *    Initial revision
 *
 */

#include "uupcmoah.h"
#include "smtpserv.h"
#include "smtpnetw.h"

RCSID("$Id: smtpserv.c 1.2 1997/11/21 18:15:18 ahd Exp $");

currentfile();

/*--------------------------------------------------------------------*/
/*       f l a g R e a d y C l i e n t s                              */
/*                                                                    */
/*       Perform select to determine what sockets are ready,          */
/*       waiting if needed                                            */
/*--------------------------------------------------------------------*/

KWBoolean
flagReadyClients( SMTPClient *master )
{
   static const char mName[] = "flagReadySockets";
   SMTPClient *current = master;

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
         printmsg( 4, "%s: Client %d has reached EOF",
                      mName,
                      getClientSequence( current ));
         setClientProcess( current, KWTrue );
      }
      else if ( isClientIgnored( current ))
      {
         printmsg( 4, "%s: Client %d ignored",
                      mName,
                      getClientSequence( current ));

      } /* if ( isClientIgnored( current )) */
      else if ( ! isClientValid( current ))
      {
         printmsg( 5, "%s: Client %d invalid",
                      mName,
                      getClientSequence( current ));
         setClientProcess( current, KWTrue );
      }
      else if ( getClientBufferedData( current ) )
         setClientProcess( current, KWTrue );
      else if ( getClientHandle( current ) < 0 )
      {
         printmsg( 4, "%s: Client %d has invalid handle %d",
                      mName,
                      getClientSequence( current ),
                      getClientHandle( current ));
         panic();
      } /* if ( isClientValid( current )) */

      current = current->next;

   } while( current );

/*--------------------------------------------------------------------*/
/*          Actually select the sockets and return to caller          */
/*--------------------------------------------------------------------*/

   return selectReadySockets( master );

/*--------------------------------------------------------------------*/
/*                   Update list of sockets to process                */
/*--------------------------------------------------------------------*/

} /* flagReadyClients */

/*--------------------------------------------------------------------*/
/*       t i m e o u t C l i e n t s                                  */
/*                                                                    */
/*       Update the status of all clients which have been idle too    */
/*       long                                                         */
/*--------------------------------------------------------------------*/

void
timeoutClients( SMTPClient *current )
{
   static const char mName[] = "timeoutClients";

   while( current != NULL )
   {
      if ( isClientTimedOut( current ))
      {
         printmsg(0, "%s: Client %d has timed out",
                  mName,
                  getClientSequence( current ));
         setClientMode( current, SM_TIMEOUT );

      } /* if ( isClientTimedOut( current )) */

      current = current->next;

   } /* while( current != NULL ) */

} /* timeoutClients */

/*--------------------------------------------------------------------*/
/*       p r o c e s s R e a d y C l i e n t s                        */
/*                                                                    */
/*       Process clients that have ready sockets                      */
/*--------------------------------------------------------------------*/

KWBoolean
processReadyClients( SMTPClient *current )
{
   while ( current != NULL )
   {
      if ( getClientProcess( current ))
      {
         processClient( current );
         setClientProcess( current, KWFalse );
      }

      current = current->next;
   }

   return KWTrue;

} /* processReadyClients */

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

   printmsg( (freed > 0) ? 4 : 8,
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
         count ++;
      }
      current = current->next;
   }

   if ( count )
      dropTerminatedClients( master ); /* Terminate active clients   */

   dropTerminatedClients( master ); /* Free all remaining clients    */

/*--------------------------------------------------------------------*/
/*                   Drop the master client itself                    */
/*--------------------------------------------------------------------*/

   freeClient( master );

} /* dropAllClients */
