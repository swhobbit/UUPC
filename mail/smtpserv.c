/*--------------------------------------------------------------------*/
/*       s m t p s e r v . c                                          */
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
 *    $Id: SMTPSERV.C 1.10 1998/03/08 23:10:20 ahd Exp $
 *
 *    $Log: SMTPSERV.C $
 *    Revision 1.10  1998/03/08 23:10:20  ahd
 *    Better UUXQT support
 *
 *    Revision 1.9  1998/03/01 19:40:48  ahd
 *    First compiling POP3 server which accepts user id/password
 *
 *    Revision 1.8  1998/03/01 01:31:59  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1997/11/28 23:11:38  ahd
 *    Additional SMTP auditing, normalize formatting, more OS/2 SMTP fixes
 *
 *    Revision 1.6  1997/11/28 04:52:10  ahd
 *    Initial UUSMTPD OS/2 support
 *
 *    Revision 1.5  1997/11/26 03:34:11  ahd
 *    Correct SMTP timeouts, break out protocol from rest of daemon
 *
 *    Revision 1.4  1997/11/25 05:05:06  ahd
 *    More robust SMTP daemon
 *
 *    Revision 1.3  1997/11/24 02:52:26  ahd
 *    First working SMTP daemon which delivers mail
 *
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
#include "execute.h"

RCSID("$Id: SMTPSERV.C 1.10 1998/03/08 23:10:20 ahd Exp $");

currentfile();

/*--------------------------------------------------------------------*/
/*       f l a g R e a d y C l i e n t L i s t                        */
/*                                                                    */
/*       Perform select to determine what sockets are ready,          */
/*       waiting if needed                                            */
/*--------------------------------------------------------------------*/

KWBoolean
flagReadyClientList(SMTPClient *master)
{
   static const char mName[] = "flagReadyClientList";
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
      printmsg(8,"%s: Processing client %d, handle %d, "
                  "mode 0x%04x, %d bytes buffered",
                  mName,
                  getClientSequence(current),
                  getClientHandle(current),
                  getClientMode(current),
                  getClientBufferedData(current));
#endif

      if (isClientEOF(current))
      {
         printmsg(4, "%s: Client %d has reached EOF",
                      mName,
                      getClientSequence(current));
         setClientProcess(current, KWTrue);
      }
      else if (isClientIgnored(current))
      {
#ifdef UDEBUG
         printmsg(9, "%s: Client %d ignored",
                      mName,
                      getClientSequence(current));
#endif

      } /* if (isClientIgnored(current)) */
      else if (! isClientValid(current))
      {
         printmsg(4, "%s: Client %d invalid",
                      mName,
                      getClientSequence(current));
         setClientMode( current, SM_DELETE_PENDING );
         setClientProcess(current, KWTrue);
      }
      else if (getClientReady(current))
         setClientProcess(current, KWTrue);
      else if (getClientBufferedData(current))
         setClientProcess(current, KWTrue);
      else if (getClientHandle(current) == INVALID_SOCKET)
      {
         printmsg(0, "%s: Client %d has invalid handle %d",
                      mName,
                      getClientSequence(current),
                      getClientHandle(current));
         panic();
      } /* if (isClientValid(current)) */

      current = current->next;

   } while(current);

/*--------------------------------------------------------------------*/
/*          Actually select the sockets and return to caller          */
/*--------------------------------------------------------------------*/

   return selectReadySockets(master);

} /* flagReadyClientList */

/*--------------------------------------------------------------------*/
/*       t i m e o u t C l i e n t L i s t                            */
/*                                                                    */
/*       Update the status of all clients which have been idle too    */
/*       long                                                         */
/*--------------------------------------------------------------------*/

void
timeoutClientList(SMTPClient *current)
{
   static const char mName[] = "timeoutClientList";

   while(current != NULL)
   {
      if (isClientTimedOut(current))
      {
         printmsg(0, "%s: Client %d has timed out",
                  mName,
                  getClientSequence(current));
         setClientMode(current, SM_TIMEOUT);
         setClientProcess(current, KWTrue);

      } /* if (isClientTimedOut(current)) */

      current = current->next;

   } /* while(current != NULL) */

} /* timeoutClientList */

/*--------------------------------------------------------------------*/
/*       p r o c e s s R e a d y C l i e n t L i s t                  */
/*                                                                    */
/*       Process clients that have ready sockets                      */
/*--------------------------------------------------------------------*/

KWBoolean
processReadyClientList(SMTPClient *current)
{
   while (current != NULL)
   {
      if (getClientProcess(current))
      {
         setClientProcess(current, KWFalse);
         processClient(current);
      }

      current = current->next;
   }

   return KWTrue;

} /* processReadyClientList */

/*--------------------------------------------------------------------*/
/*       d r o p T e r m i n a t e d C l i e n t L i s t              */
/*                                                                    */
/*       Perform all clean-up processing for clients which            */
/*       are no longer valid                                          */
/*--------------------------------------------------------------------*/

void
dropTerminatedClientList(SMTPClient *current, KWBoolean runUUXQT )
{

   static const char mName[] = "dropTerminatedClientList";
   int freed = 0;
   int total = 0;
   KWBoolean needUUXQT = KWFalse;

   while(current != NULL)
   {
      SMTPClient *next = current->next;
      total++;

      if (! isClientValid(current))
      {
         if ( getClientQueueRun(current))
            needUUXQT = KWTrue;

         freeClient(current);
         freed++;
      }

      current = next;
   }

   printmsg((freed > 0) ? 4 : 8,
            "%s: freed %d of %d client connections.",
            mName,
            freed,
            total);

   if ( needUUXQT && runUUXQT )
      executeQueue();

} /* dropTerminatedClientList */

/*--------------------------------------------------------------------*/
/*       e x e c u t e Q u e u e                                      */
/*                                                                    */
/*       Run UUXQT in background for local host                       */
/*--------------------------------------------------------------------*/

void
executeQueue( void )
{
   char buf[100];

   printmsg(1,"executeQueue: Spawning UUXQT");
   sprintf( buf, "-s %s -x %d", E_nodename, debuglevel );
   execute( "uuxqt", buf, NULL, NULL, KWFalse, KWFalse );
}

/*--------------------------------------------------------------------*/
/*       d r o p A l l C l i e n t L i s t                            */
/*                                                                    */
/*       Drop all clients, including the master client, from the      */
/*       list of clients to process                                   */
/*--------------------------------------------------------------------*/

void
dropAllClientList(SMTPClient *master, KWBoolean runUUXQT)
{
   static const char mName[] = "dropAllClientList";
   SMTPClient *current;
   int count = 0;

   printmsg(1,"%s: Dropping all clients prior to program termination.",
               mName);

   dropTerminatedClientList(master->next, runUUXQT);

   current = master->next;

   while(current != NULL)
   {

      if (isClientValid(current))
      {
         setClientMode(current, SM_EXITING);
         processClient(current);
         count ++;
      }
      current = current->next;
   }

   if (count)
   {
      /* Terminate active clients*/
      dropTerminatedClientList(master->next, runUUXQT);
   }

   /* Free all remaining clients */
   dropTerminatedClientList(master->next, runUUXQT);

/*--------------------------------------------------------------------*/
/*                   Drop the master client itself                    */
/*--------------------------------------------------------------------*/

   freeClient(master);

} /* dropAllClientList */
