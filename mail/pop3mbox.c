/*--------------------------------------------------------------------*/
/*       p o p 3 l w c . c                                            */
/*                                                                    */
/*       Light-weight POP3 server verb processors                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by the   */
/*       UUPC/extended license agreement.                             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: pop3lwc.c 1.1 1998/03/01 19:42:17 ahd Exp $
 *
 *       Revision History:
 *       $Log: pop3lwc.c $
 *       Revision 1.1  1998/03/01 19:42:17  ahd
 *       Initial revision
 *
 *
 */

/*--------------------------------------------------------------------*/
/*                           Include files                            */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include "pop3mbox.h"
#include "mail.h"
#include "smtpnetw.h"

#include "timestmp.h"
#include "arpadate.h"
#include "deliver.h"
#include "errno.h"

/*--------------------------------------------------------------------*/
/*                            Global constants                        */
/*--------------------------------------------------------------------*/

RCSID("$Id: pop3lwc.c 1.1 1998/03/01 19:42:17 ahd Exp $");

currentfile();

static const char sep[] = MESSAGESEP;
static const char uidl[] = UIDL_HEADER;

/*--------------------------------------------------------------------*/
/*       n e w P o p M e s s a g e                                    */
/*                                                                    */
/*       Intialize the definition of a message in the POP             */
/*       mailbox and add it to our queue of such messages.            */
/*--------------------------------------------------------------------*/

static
MailMessage *
newPopMessage(SMTPClient *client,
              MailMessage *previous,
              const long position)
{
   MailMessage *current;

   /* Allocate and initialize the message structure */
   current = malloc(sizeof *current);
   checkref(current);
   memset(current, 0, sizeof *current);

   /* Update ending of previous box, or start a new queue */
   if (previous != NULL)
   {
      previous->next = current;
      previous->endPosition = position - 1;

      if (previous->startBodyPosition == 0)
          previous->startBodyPosition = previous->endPosition;
   }
   else
      client->transaction->top = current;

   /* Initialize information for our new box */
   current->sequence = ++(client->transaction->messageCount);
   current->startPosition = position;
   current->previous = previous;

   /* Return our newly created object */
   return current;

} /* newPopMessage */

/*--------------------------------------------------------------------*/
/*       p o p B o x L o a d                                          */
/*                                                                    */
/*       Load the user mailbox into an working temporary file         */
/*--------------------------------------------------------------------*/

KWBoolean
popBoxLoad(SMTPClient *client)
{
   static const char mName[] = "popBoxLoad";
   MailMessage *current = NULL;
   long position = -1;
   long length;
   char buffer[BUFSIZ];

   client->transaction->mailboxStream =
                  FOPEN(client->transaction->mailboxName,
                         "r+",
                         TEXT_MODE);

   /* Just treat file not existing as no mail */
   if (client->transaction->mailboxStream == NULL)
   {
      if ( errno == EACCES )        /* Unless it's access problem */
         return KWFalse;
      else
         return KWTrue;
   }

   length = fileno(client->transaction->mailboxStream);

   client->transaction->imf = imopen(length, IMAGE_MODE);

   if (client->transaction->imf == NULL)
   {
      printerr("imopen");
      return KWFalse;
   }

   position = imtell(client->transaction->imf);

/*--------------------------------------------------------------------*/
/*              Loop to copy and parse the input mailbox              */
/*--------------------------------------------------------------------*/

   while (fgets(buffer,
                sizeof buffer,
                client->transaction->mailboxStream) != NULL)
   {
      size_t bytes = strlen(buffer);

      position = imtell(client->transaction->imf);

      /* Handle the delimiter line which flags a new message */
      if (equaln(buffer, sep, strlen(sep)))
      {
         /* Add new message into the message queue */
         current = newPopMessage(client, current, position);

         /* Don't write or count the bytes in the header line */
         continue;
      }

      /* Validate that we are actually in a message */
      if (current == NULL)
      {
         printmsg(0, "%s: Corrupt mailbox %s, "
                     "did not begin with delimiter line",
                     mName,
                     client->transaction->mailboxName);
         return KWFalse;
      }

      current->octets += bytes + 1; /* UNIX is LF, POP3 is CR/LF  */

      /* Now write it out, with error checking */
      if (imwrite(buffer,
                  sizeof(char),
                  bytes,
                  client->transaction->imf) != bytes)
      {
         printmsg(0, "Error including new mail into %s",
                     client->transaction->mailboxName);
         printerr("imfile");
         return KWFalse;
      }

      /* Perform limited header parsing */
      if (current->startBodyPosition == 0)
      {
         if (equal(buffer, "\n"))
            current->startBodyPosition = position;
         else if (equaln(buffer, uidl, sizeof uidl - 1))
         {
            /* Destructively parse the buffer for the UIDL */
            char *token = strtok(buffer + sizeof uidl - 1, WHITESPACE);

            if (token != NULL)
            {
               /* We take the last uidl we find */
               if (current->uidl != NULL)
                  free( current->uidl );

               current->uidl = strdup(token);
               checkref(current->uidl);
            }
            else
               printmsg(0, "%s: %s No UIDL found on %s line for "
                           "message %d",
                           mName,
                           client->transaction->mailboxName,
                           uidl,
                           current->sequence);

         } /* else if (equaln(buffer, uidl, sizeof uidl - 1)) */

      } /* if (current->startBodyPosition == 0) */

   } /* while */

/*--------------------------------------------------------------------*/
/*                       Check for input errors                       */
/*--------------------------------------------------------------------*/

   if (ferror(client->transaction->mailboxStream))
   {
      printerr(client->transaction->mailboxName);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*        Handle final accounting for the last message, if any        */
/*--------------------------------------------------------------------*/

   if (current != NULL)
   {
      current->endPosition = imtell(client->transaction->imf);
      if (current->startBodyPosition == 0)
         current->startBodyPosition = current->endPosition;
   }

/*--------------------------------------------------------------------*/
/*                  Success!  Report same to caller                   */
/*--------------------------------------------------------------------*/

   return KWTrue;

} /* popBoxLoad */

/*--------------------------------------------------------------------*/
/*       p o p B o x G e t                                            */
/*                                                                    */
/*       Return pointer to a message in the POP mailbox               */
/*--------------------------------------------------------------------*/

MailMessage *
popBoxGet( MailMessage *current, const long sequence )
{
   while( current != NULL )
   {
      if ( current->sequence == sequence )
         return current;

      current = current->next;
   }

   /* Didn't find the message, report failure */
   return NULL;

} /* popBoxGet */

/*--------------------------------------------------------------------*/
/*       g e t P o p M e s s a g e N e x t                            */
/*                                                                    */
/*       Iterate to the next undeleted message                        */
/*--------------------------------------------------------------------*/

MailMessage *
getPopMessageNext( MailMessage *current )
{
   if ( current == NULL )
      return NULL;

   current = current->next;

   while( current != NULL )
   {
      if ( ! current->deleted )
         return current;

      current = current->next;

   } /* while( current != NULL ) */

   /* We did not find an undeleted message, report failure */
   return NULL;

} /* getPopMessageNext */

/*--------------------------------------------------------------------*/
/*       p o p B o x U n d e l e t e                                  */
/*                                                                    */
/*       Restore status of to "not deleted" of all messages           */
/*--------------------------------------------------------------------*/

int
popBoxUndelete( MailMessage *current )
{
   int count = 0;

   while( current != NULL )
   {
      if ( current->deleted )
      {
         current->deleted = KWFalse;
         count++;
      }
      current = current->next;
   }

   return count;

} /* popBoxUndelete */

/*--------------------------------------------------------------------*/
/*       g e t M e s s a g e O c t e t C o u n t                      */
/*                                                                    */
/*       Report the number of bytes in the POP 3 mailbox              */
/*--------------------------------------------------------------------*/

long
getMessageOctetCount( MailMessage *current,
                      long *messageCountPtr )
{
   long localMessageCount = 0;
   long octets = 0;

   while( current != NULL )
   {
      if ( !current->deleted )
      {
         localMessageCount ++;
         octets += current->octets;
      }

      current = current->next;
   }

   if ( messageCountPtr != NULL )
      *messageCountPtr = localMessageCount;

   return octets;

}  /* getMessageOctetCount */

/*--------------------------------------------------------------------*/
/*       p o p B o x U I D L                                          */
/*                                                                    */
/*       Retrieve UIDL for a message                                  */
/*--------------------------------------------------------------------*/

char *
popBoxUIDL( MailMessage *current )
{
   char buffer[20];
   if ( current->uidl != NULL )
      return current->uidl;

   /* Build a fake UIDL */
   sprintf( buffer, "%d.%d", current->sequence, current->octets );

   current->fakeUIDL = KWTrue;
   current->uidl = strdup(buffer);
   checkref(current->uidl);
   return current->uidl;

} /* popBoxUIDL */

/*--------------------------------------------------------------------*/
/*       c l e a n u p M a i l b o x                                  */
/*                                                                    */
/*                                                                    */
/*       Clean up in-memory mailbox structures                        */
/*--------------------------------------------------------------------*/

void
cleanupMailbox( MailMessage *current )
{
   while( current != NULL )
   {
      MailMessage *next = current->next;

      if ( current->uidl != NULL )
      {
         free( current->uidl );
         current->uidl = NULL;
      }

      memset( current, 0, sizeof *current );
      free( current );
      current = next;

   } /* while( current != NULL ) */

} /* cleanupMailbox */
