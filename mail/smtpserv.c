/*--------------------------------------------------------------------*/
/*       s m t p s e r v . c                                          */
/*                                                                    */
/*       SMTP server support routines for clients                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1999 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpserv.c 1.21 1999/01/04 03:54:27 ahd Exp $
 *
 *    $Log: smtpserv.c $
 *    Revision 1.21  1999/01/04 03:54:27  ahd
 *    Annual copyright change
 *
 *    Revision 1.20  1998/11/24 14:10:23  ahd
 *    Add missing header for title.h
 *
 *    Revision 1.19  1998/09/08 23:21:22  ahd
 *    Add setting of title for major operations
 *
 *    Revision 1.18  1998/07/27 01:03:54  ahd
 *    Spin log off BEFORE invoking UUXQT to prevent problems
 *    with delete while child program is running
 *
 * Revision 1.17  1998/05/15  03:13:48  ahd
 * When entering states where no read is required, set
 * flag to skip the read to avoid hanging.
 *
 *    Revision 1.16  1998/05/11 13:55:28  ahd
 *    When client times out, don't try to read buffer
 *
 *    Revision 1.15  1998/05/11 01:20:48  ahd
 *    Spin off log on regular basis
 *
 *    Revision 1.14  1998/04/27 01:45:15  ahd
 *    *** empty log message ***
 *
 *    Revision 1.13  1998/04/24 03:30:13  ahd
 *    Use local buffers, not client->transmit.buffer, for output
 *    Rename receive buffer, use pointer into buffer rather than
 *         moving buffered data to front of buffer every line
 *    Restructure main processing loop to give more priority
 *         to client processing data already buffered
 *    Add flag bits to client structure
 *    Add flag bits to verb tables
 *
 *    Revision 1.12  1998/04/22 01:19:54  ahd
 *    Performance improvements for SMTPD data mode
 *
 *    Revision 1.11  1998/04/08 11:35:35  ahd
 *    CHange error processing for bad sockets
 *
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
#include "title.h"
#include "logger.h"

RCSID("$Id: smtpserv.c 1.21 1999/01/04 03:54:27 ahd Exp $");

/*--------------------------------------------------------------------*/
/*       f l a g R e a d y C l i e n t L i s t                        */
/*                                                                    */
/*       Perform select to determine what sockets are ready,          */
/*       waiting if needed                                            */
/*--------------------------------------------------------------------*/

KWBoolean
flagReadyClientList(SMTPClient *top)
{
   static const char mName[] = "flagReadyClientList";
   SMTPClient *current = top;

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

      if (debuglevel >= 6)
         printmsg(6,"%s: Processing client %d, handle %d, "
                     "mode 0x%04x",
                     mName,
                     getClientSequence(current),
                     getClientHandle(current),
                     getClientMode(current));
#endif

      if (getClientProcess(current))
      {
         /* No operation */
#ifdef UDEBUG
         printmsg(9, "%s: Client %d Already eligible for processing",
                      mName,
                      getClientSequence(current));
#endif
      }
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
         setClientMode(current, SM_DELETE_PENDING );
         setClientFlag(current, SF_NO_READ);
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

   return selectReadySockets(top);

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
         setClientFlag(current, SF_NO_READ);
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
      while (getClientProcess(current))
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
   int masters = 0;
   KWBoolean needUUXQT = KWFalse;

   while(current != NULL)
   {
      SMTPClient *next = current->next;
      total++;

      /* Keep count of master sockets in list */
      if (getClientMode(current) == SM_MASTER)
         masters++;

      if (! isClientValid(current))
      {
         if (getClientQueueRun(current))
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

/*--------------------------------------------------------------------*/
/*       Consider spinning the log off if we dropped our last         */
/*       active client                                                */
/*--------------------------------------------------------------------*/

   if (freed && ((freed+masters) == total))
   {
      static time_t nextSpin = 0;
      time_t now;

      time(&now);
      if (nextSpin < now)
      {
         /* Spin off the log if we are initialized */
         if (nextSpin > 0)
         {
            printmsg(0,"Spinning off log file");
            openlog(NULL);
            printmsg(4,"Started new log file");
         }

         nextSpin = now + 3600;     /* Start clock ticking        */
      }
      else
         nextSpin -= 200;           /* More clients, more often
                                       we spin                    */

      setTitle("Waiting for connection");
   }

   /* Run UUXQT as needed, we do this after spinning the log
      any handles left open for UUXQT don't screw up log deletion */
   if (needUUXQT && runUUXQT )
      executeQueue();

} /* dropTerminatedClientList */

/*--------------------------------------------------------------------*/
/*       e x e c u t e Q u e u e                                      */
/*                                                                    */
/*       Run UUXQT in background for local host                       */
/*--------------------------------------------------------------------*/

void
executeQueue(void)
{
   char buf[100];

   printmsg(1,"executeQueue: Spawning UUXQT");
   sprintf(buf, "-s %s -x %d", E_nodename, debuglevel);
   execute("uuxqt", buf, NULL, NULL, KWFalse, KWFalse);
}

/*--------------------------------------------------------------------*/
/*       d r o p A l l C l i e n t L i s t                            */
/*                                                                    */
/*       Drop all clients, including the master client, from the      */
/*       list of clients to process                                   */
/*--------------------------------------------------------------------*/

void
dropAllClientList(SMTPClient *top, KWBoolean runUUXQT)
{
   static const char mName[] = "dropAllClientList";
   SMTPClient *current;
   int count = 0;

   printmsg(1,"%s: Dropping all clients prior to program termination.",
               mName);

   dropTerminatedClientList(top->next, runUUXQT);

   current = top->next;

   while(current != NULL)
   {

      if (isClientValid(current))
      {
         setClientMode(current, SM_EXITING);
         setClientFlag(current, SF_NO_READ);
         processClient(current);
         count ++;
      }
      current = current->next;
   }

   if (count)
   {
      /* Terminate active clients*/
      dropTerminatedClientList(top->next, runUUXQT);
   }

   /* Free all remaining clients */
   dropTerminatedClientList(top->next, runUUXQT);

/*--------------------------------------------------------------------*/
/*                   Drop the master client itself                    */
/*--------------------------------------------------------------------*/

   freeClient(top);

} /* dropAllClientList */
