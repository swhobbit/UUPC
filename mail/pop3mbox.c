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
 *       $Id: pop3mbox.c 1.2 1998/03/03 07:36:28 ahd v1-12v $
 *
 *       Revision History:
 *       $Log: pop3mbox.c $
 *       Revision 1.2  1998/03/03 07:36:28  ahd
 *       Add deletion support
 *
 *       Revision 1.1  1998/03/03 03:51:53  ahd
 *       Initial revision
 *
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

RCSID("$Id: pop3mbox.c 1.2 1998/03/03 07:36:28 ahd v1-12v $");

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

   while (fgets(client->transmit.data,
                client->transmit.length,
                client->transaction->mailboxStream) != NULL)
   {
      size_t bytes = strlen(client->transmit.data);

      position = imtell(client->transaction->imf);

      /* Handle the delimiter line which flags a new message */
      if (equaln(client->transmit.data, sep, strlen(sep)))
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
      if (imwrite(client->transmit.data,
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
         if (equal(client->transmit.data, "\n"))
            current->startBodyPosition = position;
         else if (equaln(client->transmit.data, uidl, sizeof uidl - 1))
         {
            /* Destructively parse the buffer for the UIDL */
            char *token = strtok(client->transmit.data +
                                    sizeof uidl - 1,
                                 WHITESPACE);

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

         } /* else if (equaln(client->transmit.data, ... )) */

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
/*       p o p M e s s a g e U n l o a d                              */
/*                                                                    */
/*       Unload messages back to permanent storage                    */
/*--------------------------------------------------------------------*/

static KWBoolean
popMessageUnload( SMTPClient *client,
                  MailMessage *current )
{
   /* Go to the beginning of the message */
   if (imseek(client->transaction->imf,
               current->startPosition,
               SEEK_SET) == -1)
   {
      printerr("imfile");
      return KWFalse;
   }

   /* Delimit the message */
   fprintf( client->transaction->mailboxStream,"%s", sep);

   /* Loop for entire input message and copy it */
   for (;;)
   {
      long position = imtell(client->transaction->imf);
      size_t length;

      if (position < 0)
         break;

      if (position >= current->endPosition)
         break;

      if (imgets(client->transmit.data,
                 client->transmit.length - 2,
                 client->transaction->imf) == NULL)
         break;

      /* If we need to write our UIDL, write at first empty line,
         which defines the end of the header */
      if (current->fakeUIDL && equal(client->transmit.data, "\n"))
      {
         current->fakeUIDL = KWFalse;  /* Don't write it twice! */

         fprintf( client->transaction->mailboxStream,
                  "%s %s\n",
                  uidl,
                  current->uidl );
      }

      /* Now write out the input line */
      length = strlen(client->transmit.data);

      if (fwrite(client->transmit.data,
                  sizeof(char),
                  length,
                  client->transaction->mailboxStream) != length)
      {
         printerr( client->transaction->mailboxName );
         return KWFalse;
      }

   } /* for (;;) */

   /* Verify we didn't have an input problem */
   if (imerror(client->transaction->imf))
   {
      printerr( client->transaction->mailboxName );
      return KWFalse;
   }

   /* Return success */
   return KWTrue;

} /* popMessageUnload */

/*--------------------------------------------------------------------*/
/*       p o p B o x U n l o a d                                      */
/*                                                                    */
/*       Write out a POP3 mailbox to disk                             */
/*--------------------------------------------------------------------*/

KWBoolean
popBoxUnload( SMTPClient *client )
{
   KWBoolean success = KWTrue;
   MailMessage *current;

   /* Handle degenerate case of the entire mailbox being empty */
   if (bflag[F_PURGE] &&
       (getMessageOctetCount(client->transaction->top, NULL) == 0))
   {

      /* Close and remove as fast we can */
      imclose( client->transaction->imf );
      client->transaction->imf = NULL;

      fclose(client->transaction->mailboxStream);
      client->transaction->mailboxStream = NULL;
      REMOVE(client->transaction->mailboxName);

      printmsg(1,"Empty mail box %s has been deleted.",
               client->transaction->mailboxName);

      return KWTrue;

   } /* if (bflag[F_PURGE] && ... */

   /* Truncate the mailbox in order to rewrite it */
   client->transaction->mailboxStream =
                     freopen(client->transaction->mailboxName,
                             "wt",
                             client->transaction->mailboxStream);

   if ( client->transaction->mailboxStream == NULL )
   {
      printerr( client->transaction->mailboxName );
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*            Loop through the messages and write them out            */
/*--------------------------------------------------------------------*/

   current = client->transaction->top;

   while((current != NULL) && success)
   {
      if ( ! popBoxIsDeleted( current ))
         success = popMessageUnload( client, current );

      current = current->next;

   } /* while((current != NULL) && success) */

/*--------------------------------------------------------------------*/
/*          Close up shop and return with saved error status          */
/*--------------------------------------------------------------------*/

   imclose( client->transaction->imf );
   client->transaction->imf = NULL;

   fclose(client->transaction->mailboxStream);
   client->transaction->mailboxStream = NULL;

   return success;

} /* popBoxUnload */

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
   sprintf( buffer, "<%x.%x.%d>",
                   time(NULL),
                   current->octets,
                   current->sequence);

   current->fakeUIDL = KWTrue;
   current->uidl = strdup(buffer);
   checkref(current->uidl);
   return current->uidl;

} /* popBoxUIDL */

/*--------------------------------------------------------------------*/
/*       c l e a n u p M a i l b o x                                  */
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
