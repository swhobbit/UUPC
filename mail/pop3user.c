/*--------------------------------------------------------------------*/
/*       p o p 3 u s e r . c                                          */
/*                                                                    */
/*       POP3 specific heavy processes                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2001 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by the   */
/*       UUPC/extended license agreement.                             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: pop3user.c 1.19 2000/10/14 12:40:44 ahd Exp $
 *
 *       Revision History:
 *       $Log: pop3user.c $
 *       Revision 1.19  2000/10/14 12:40:44  ahd
 *       Correctly strip byte stuffed periods from ANY line beginning with
 *       a period (.).
 *
 *       Revision 1.18  2000/05/12 12:35:45  ahd
 *       Annual copyright update
 *
 *       Revision 1.17  1999/01/08 02:21:01  ahd
 *       Convert currentfile() to RCSID()
 *
 *       Revision 1.16  1999/01/04 03:54:27  ahd
 *       Annual copyright change
 *
 *       Revision 1.15  1998/11/04 01:59:55  ahd
 *       Prevent buffer overflows when processing UIDL lines
 *       in POP3 mail.
 *       Add internal sanity checks for various client structures
 *       Convert various files to CR/LF from LF terminated lines
 *
 * Revision 1.14  1998/05/15  03:13:48  ahd
 * Don't attempt to read size of mailbox at QUIT if no mailbox
 * initialized
 *
 *       Revision 1.13  1998/05/11 13:55:01  ahd
 *       Correct compile error from previous fix
 *
 *       Revision 1.12  1998/05/11 01:20:48  ahd
 *       Clarify mailbox deleted status at cleanup
 *
 *       Revision 1.11  1998/04/24 03:30:13  ahd
 *       Use local buffers, not client->transmit.buffer, for output
 *       Rename receive buffer, use pointer into buffer rather than
 *            moving buffered data to front of buffer every line
 *       Restructure main processing loop to give more priority
 *            to client processing data already buffered
 *       Add flag bits to client structure
 *       Add flag bits to verb tables
 *
 *       Revision 1.10  1998/04/19 15:30:08  ahd
 *       Correctly handle quoting periods at the start of lines
 *       Trap network errors when sending messages to remote clients
 *       Use buffer large enough for sending most messages in one transmit
 *
 *       Revision 1.9  1998/04/08 11:32:07  ahd
 *       Correct processing of continued lines when sending messages
 *       to POP client
 *
 *       Revision 1.8  1998/03/16 06:40:49  ahd
 *       Buffer up lines for sending
 *
 *       Revision 1.7  1998/03/09 01:18:19  ahd
 *       Normalize function names
 *
 *       Revision 1.6  1998/03/08 23:07:12  ahd
 *       Fully buffer transmitted text of messages
 *
 *       Revision 1.5  1998/03/08 07:47:46  ahd
 *       Correct message text for msg retrieval
 *
 *       Revision 1.4  1998/03/06 06:51:28  ahd
 *       Add commands to make Netscape happy
 *
 *       Revision 1.3  1998/03/03 07:36:28  ahd
 *       Add deletion support
 *
 *       Revision 1.2  1998/03/03 03:53:54  ahd
 *       Routines to handle messages within a POP3 mailbox
 *
 *       Revision 1.1  1998/03/01 19:42:17  ahd
 *       Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                           Include files                            */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include <limits.h>

#include "pop3mbox.h"
#include "pop3user.h"
#include "smtpnetw.h"
#include "mailnewm.h"
#include "expath.h"

/*--------------------------------------------------------------------*/
/*                            Global files                            */
/*--------------------------------------------------------------------*/

RCSID("$Id: pop3user.c 1.19 2000/10/14 12:40:44 ahd Exp $");

/*--------------------------------------------------------------------*/
/*       s el e c t P o p M e s s a g e                               */
/*                                                                    */
/*       Return pointer to specified POP message                      */
/*--------------------------------------------------------------------*/

static MailMessage *
selectPopMessage(SMTPClient *client, const char *number)
{
   char xmitBuf[XMIT_LENGTH];
   long sequence = atol(number);
   char *errorText = NULL;
   MailMessage *current;

   if (sequence < 1)
      errorText = "Operand must be numeric greater than zero";
   else if (sequence > client->transaction->messageCount)
      errorText = "Operand must be less than number messages in mailbox";
   else if ((current = popBoxGet(client->transaction->top,
                                  sequence)) == NULL)
      errorText = "Internal Error, cannot retrieve message";
   else if (popBoxIsDeleted(current))
      errorText = "Cannot access message pending delete";
   else
      return current;

/*--------------------------------------------------------------------*/
/*       We have an error, report it and return NULL to caller        */
/*--------------------------------------------------------------------*/

   sprintf(xmitBuf,
            "Cannot process message %s of %ld for user %s, error: %s",
            number,
            client->transaction->messageCount,
            client->clientName,
            errorText);
   SMTPResponse(client, PR_ERROR_GENERIC, xmitBuf);

   return NULL;

} /* selectPopMessage */

/*--------------------------------------------------------------------*/
/*       w r i t e P o p M e s s a g e                                */
/*                                                                    */
/*       Write actual message text to client                          */
/*--------------------------------------------------------------------*/

void
writePopMessage(SMTPClient *client,
                 MailMessage *current,
                 long bodyLines)
{
   static const char mName[] = "writePopMessage";
   static const char crlf[] = "\r\n";

   KWBoolean networkError = KWFalse;   /* We're dead, Jim            */
   KWBoolean continued = KWFalse;      /* Previous line incomplete   */
   KWBoolean wasPeriods = KWFalse;     /* Previous incomplete line
                                          all periods                */
   long octets = 0;

   const long bufferLength = min(32, (current->octets / 1024) + 1) * 1024;
   char *buffer;
   long bufferUsed = 0;

   if (imseek(client->transaction->imf,
              current->startPosition,
              SEEK_SET) == -1)
   {
      printerr("imfile");
      SMTPResponse(client, PR_ERROR_GENERIC, "Seek failed");
      return;
   }

   /* Allocate our buffer for processing */
   buffer = malloc(bufferLength);
   checkref(buffer);

   /* Send the header line announcing a message follows */
   sprintf(buffer, "%ld Octets", current->octets);
   if (!SMTPResponse(client, PR_OK_GENERIC, buffer))
   {
      free(buffer);
      return;
   }

   /* Loop for entire header and as many lines of body as required */
   for (;;)
   {
      KWBoolean incomplete = KWFalse;
#ifdef UDEBUG
      KWBoolean quotedPeriod = KWFalse;
#endif
      long position = imtell(client->transaction->imf);
      char *linePointer = buffer + bufferUsed;
      size_t length;

      if (position < 0)
         break;

      if (position >= current->endPosition)
         break;

      if ((position >= current->startBodyPosition) && (bodyLines-- < 0))
         break;

      /* - 3 == Leave room for period, carriage-return, linefeed */
      if (imgets(linePointer,
                 bufferLength - bufferUsed - 3,
                 client->transaction->imf) == NULL)
      {
         /* Jump out of loop on EOF */
         break;
      }

      length = strlen(linePointer);

      if (linePointer[length - 1] == '\n')
      {
         /* Trim off LF we found (normal case); we'll add       */
         /* an Internet standard CR/LF period below if needed.  */
         linePointer[--length] = '\0';
      }
      else
      {
         /* Line must be too long for normal use. */
         incomplete = KWTrue;
      }

      /* Unlike SMTP, POP3 only checks first octet of each line for */
      /* quoting a leading period; this makes an easy check.        */
      if (!continued && (linePointer[0] == '.'))
      {
         /*
          * Shift the entire buffer over one octet to byte stuff an
          * extra leading period.  This quotes the original leading
          * perdiod.
          */
         memmove(linePointer + 1, linePointer, length++);
         linePointer[0] = '.';

#ifdef UDEBUG
         quotedPeriod = KWTrue;
#endif
      }

#ifdef UDEBUG
      printmsg(5,"--> [%03d] %s%s%s%s",
                  bufferUsed,
                  linePointer,
                  quotedPeriod ? " (quoted period)" : "",
                  continued ? " (continued)" : "",
                  incomplete ? " (incomplete)" : "");
#endif

      bufferUsed += length;

      /* Terminate the line */
      if (! incomplete)
      {
         strcat(linePointer, crlf);

         /* Update our running total */
         bufferUsed += 2;
      }

      /* Determine if we wand to flush buffer */
      if ((bufferLength - bufferUsed) < 80)
      {
         if (SMTPResponse(client, PR_TEXT, buffer))
         {
            octets += bufferUsed;
            bufferUsed = 0;
         }
         else {
            networkError = KWTrue;
            break;
         }

      } /* if ((bufferLength - bufferUsed) < 80) */

      /* If this cycle is incomplete, we continue next cycle ... */
      continued = incomplete;

   } /* for (;;) */

/*--------------------------------------------------------------------*/
/*                     End of our loop, clean up                      */
/*--------------------------------------------------------------------*/

   /* Flush final information in buffer */
   if (!networkError && (bufferUsed > 0))
   {
      if (SMTPResponse(client, PR_TEXT, buffer))
      {
         octets += bufferUsed;
         bufferUsed = 0;
      }
      else
         networkError = KWTrue;

   } /* if (bufferUsed > 0) */

   if (imerror(client->transaction->imf))
      printerr(client->transaction->mailboxName);

   /* Drop allocated resources */
   free(buffer);

   /*
      Terminate the message with a line only consisting of a
      period (.)
    */
   if (!networkError && !SMTPResponse(client, PR_DATA, "."))
      networkError = KWTrue;

   if (networkError)
   {
      printmsg(3, "%s: Unable to send message %ld to %s "
                  "because of network error.",
                  mName,
                  current->sequence,
                  client->clientName);
   }
   else {
      printmsg(3, "%s: Sent message %ld to %s (%ld of %ld octets)",
                  mName,
                  current->sequence,
                  client->clientName,
                  octets,
                  current->octets);
   }

} /* writePopMessage */

/*--------------------------------------------------------------------*/
/*       c o m m a n d L o a d M a i l b o x                          */
/*                                                                    */
/*       Load the working copy of our POP3 mailbox                    */
/*--------------------------------------------------------------------*/

KWBoolean
commandLoadMailbox(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{
   char xmitBuf[XMIT_LENGTH];

/*--------------------------------------------------------------------*/
/*                   Create the final mailbox name                    */
/*--------------------------------------------------------------------*/

   strncpy(client->transaction->mailboxName,
            client->clientName,
            sizeof client->transaction->mailboxName);

   client->transaction->mailboxName[8] = '\0';

   if (expand_path(client->transaction->mailboxName,
                    client->transaction->userp->homedir,
                    client->transaction->userp->homedir,
                    E_mailext) == NULL)
   {
      sprintf(xmitBuf,
              "Internal error, cannot determine mailbox location for %s",
              client->clientName);

      SMTPResponse(client, PR_ERROR_GENERIC, xmitBuf);
      setClientMode(client, P3_AUTHORIZATION);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*                Get any new mail from system mailbox                */
/*--------------------------------------------------------------------*/

   if (! IncludeNew(client->transaction->mailboxName,
                      client->clientName))
   {
      sprintf(xmitBuf,
              "Internal error, unable to append new mail to mailbox %s: %s",
              client->transaction->mailboxName,
              strerror(errno));
      SMTPResponse(client, PR_ERROR_GENERIC, xmitBuf);
      setClientMode(client, P3_AUTHORIZATION);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*         Load the mailbox information, with error checking          */
/*--------------------------------------------------------------------*/

   if (! popBoxLoad(client))
   {

      sprintf(xmitBuf,
              "Internal error, unable to load mailbox %s",
              client->transaction->mailboxName);

      SMTPResponse(client, PR_ERROR_GENERIC, xmitBuf);
      setClientMode(client, P3_AUTHORIZATION);
      return KWFalse;

   } /* if (! popBoxLoad(client)) */

/*--------------------------------------------------------------------*/
/*                     Report our mailbox is open                     */
/*--------------------------------------------------------------------*/

   sprintf(xmitBuf,
           "%s has %ld message(s) (%ld octets)",
           client->clientName,
           client->transaction->messageCount,
           getMessageOctetCount(client->transaction->top, NULL));

   SMTPResponse(client,
                verb->successResponse,
                xmitBuf);

   return KWTrue;

} /* commandLoadMailbox */

/*--------------------------------------------------------------------*/
/*       c o m m a n d D E L E                                        */
/*                                                                    */
/*       Flag a command for deletion                                  */
/*--------------------------------------------------------------------*/

KWBoolean
commandDELE(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{

   MailMessage *current = selectPopMessage(client, operands[0]);

   if (current == NULL)
      return KWFalse;

   popBoxDelete(current);

   /* Flag we will need to update the mailbox at exit */
   client->transaction->rewrite = KWTrue;

   SMTPResponse(client,
                verb->successResponse,
                "Message deleted.");
   return KWTrue;

} /* commandDELE */

/*--------------------------------------------------------------------*/
/*       l i s t O n e M e s s a g e                                  */
/*                                                                    */
/*       Report size of one message                                   */
/*--------------------------------------------------------------------*/

static void
listOneMessage(SMTPClient *client,
                MailMessage *current,
                int code)
{
   char xmitBuf[XMIT_LENGTH];

   assertSMTP(client);
   assertPOP3Message(current);

   sprintf(xmitBuf, "%ld %ld",
           current->sequence,
           current->octets);
   SMTPResponse(client, code, xmitBuf);

} /* listOneMessage */

KWBoolean
commandLIST(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{
   char xmitBuf[XMIT_LENGTH];
   MailMessage *current;
   long octets;
   long messages;

/*--------------------------------------------------------------------*/
/*                Handle a single message to report on                */
/*--------------------------------------------------------------------*/

   if (operands[0] != NULL)
   {
      current = selectPopMessage(client, operands[0]);
      if (current == NULL)
         return KWFalse;
      listOneMessage(client, current, verb->successResponse);
      incrementClientMajorTransaction(client);
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*                  Report on all undeleted messages                  */
/*--------------------------------------------------------------------*/

   /* Send the header line */
   octets = getMessageOctetCount(client->transaction->top, &messages);

   sprintf(xmitBuf,
           "%ld message(s) (%ld octets)",
           messages, octets);
   SMTPResponse(client, verb->successResponse, xmitBuf);

   /* Now loop through the actual messages to report on */
   current = client->transaction->top;
   while(current != NULL)
   {
      listOneMessage(client, current, PR_DATA);
      current = getBoxPopNext(current);
   } /* while(current != NULL) */

   /* Terminate the list of messages */
   SMTPResponse(client, PR_DATA, ".");

   /* Report success to caller */
   incrementClientMajorTransaction(client);
   return KWTrue;

} /* commandLIST */

/*--------------------------------------------------------------------*/
/*       c o m m a n d Q U I T                                        */
/*                                                                    */
/*       Respond to server termination                                */
/*--------------------------------------------------------------------*/

KWBoolean
commandQUIT(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{
   char xmitBuf[XMIT_LENGTH];
   KWBoolean success = KWTrue;
   long messages = 0, octets = 0;

   if ((client->transaction == NULL) || !client->transaction->rewrite)
   {
      sprintf(xmitBuf,
               "%s Closing connection, adios",
               E_domain);
   }
   else if (! popBoxUnload( client))
   {
      success = KWFalse;
      sprintf(xmitBuf,
              "Unable to rewrite mailbox %s, it may be corrupted!",
              client->transaction->mailboxName);
   }
   else if ((octets = getMessageOctetCount(client->transaction->top, &messages)) == 0)
   {
      /* All messages were deleted */
      sprintf(xmitBuf,
              "Updated mailbox %s, deleted all (%ld) messages.",
              client->transaction->mailboxName,
              client->transaction->messageCount);
   }
   else {
      /* Some (or all) messages are left in the mailbox */
      sprintf(xmitBuf,
              "Updated mailbox %s, "
              "now contains %ld octets in %ld messages.",
              client->transaction->mailboxName,
              octets,
              messages);
   }

   SMTPResponse(client,
                success ? verb->successResponse :
                          PR_ERROR_GENERIC,
                xmitBuf);

   return KWTrue;

} /* commandQUIT */

/*--------------------------------------------------------------------*/
/*       c o m m a n d R E T R                                        */
/*                                                                    */
/*       Retrieve full text of a message                              */
/*--------------------------------------------------------------------*/

KWBoolean
commandRETR(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{
   MailMessage *current = selectPopMessage(client, operands[0]);

   if (current == NULL)
      return KWFalse;

   /* Write entire message out */
   writePopMessage(client, current, LONG_MAX);
   incrementClientMajorTransaction(client);

   return KWTrue;

} /* commandRETR */

/*--------------------------------------------------------------------*/
/*       c o m m a n d R S E T                                        */
/*                                                                    */
/*       Reset server state to allow new transaction                  */
/*--------------------------------------------------------------------*/

KWBoolean
commandRSET(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{
   popBoxUndelete(client->transaction->top);
   SMTPResponse(client, verb->successResponse, "All messages undeleted");
   return KWTrue;
}

/*--------------------------------------------------------------------*/
/*       c o m m a n d S T A T                                        */
/*                                                                    */
/*       Report number of messages and bytes in the POP mailbox       */
/*--------------------------------------------------------------------*/

KWBoolean
commandSTAT(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{
   long octets;
   long messages;
   char xmitBuf[XMIT_LENGTH];

   octets = getMessageOctetCount(client->transaction->top, &messages);

   sprintf(xmitBuf,
           "%ld %ld",
           messages, octets);
   SMTPResponse(client, verb->successResponse, xmitBuf);

   /* Report Success to caller */
   return KWTrue;

} /* commandSTAT */

/*--------------------------------------------------------------------*/
/*       c o m m a n d T O P                                          */
/*                                                                    */
/*       Print first few lines of a message                           */
/*--------------------------------------------------------------------*/

KWBoolean
commandTOP(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{
   MailMessage *current = selectPopMessage(client, operands[0]);

   if (current == NULL)
      return KWFalse;

   incrementClientMajorTransaction(client);
   writePopMessage(client, current, atol(operands[1]));

   return KWTrue;

} /* commandTOP */

/*--------------------------------------------------------------------*/
/*       i d e n t i f y O n e M e s s a g e                          */
/*                                                                    */
/*       Report size of one message                                   */
/*--------------------------------------------------------------------*/

static void
identifyOneMessage(SMTPClient *client,
                    MailMessage *current,
                    int code)
{
   char xmitBuf[XMIT_LENGTH];

   sprintf(xmitBuf, "%ld %s",
           current->sequence,
           popBoxUIDL(current));
   SMTPResponse(client, code, xmitBuf);

   /* Generation of a UIDL means we have to rewrite the mailbox */
   if (popBoxIsUpdated(current))
      client->transaction->rewrite = KWTrue;

} /* listOneMessage */

/*--------------------------------------------------------------------*/
/*       c o m m a n d U I D L                                        */
/*                                                                    */
/*       Report unique identifiers for one or more messages           */
/*--------------------------------------------------------------------*/

KWBoolean
commandUIDL(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{

   MailMessage *current;

/*--------------------------------------------------------------------*/
/*                Handle a single message to report on                */
/*--------------------------------------------------------------------*/

   if (operands[0] != NULL)
   {
      current = selectPopMessage(client, operands[0]);
      if (current == NULL)
         return KWFalse;
      identifyOneMessage(client, current, verb->successResponse);
      incrementClientMajorTransaction(client);
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*                  Report on all undeleted messages                  */
/*--------------------------------------------------------------------*/

   /* Send the header line */
   SMTPResponse(client, verb->successResponse, "unique-id list follows");

   /* Now loop through the actual messages to report on */
   current = client->transaction->top;
   while(current != NULL)
   {
      identifyOneMessage(client, current, PR_DATA);
      current = getBoxPopNext(current);
   } /* while(current != NULL) */

   /* Terminate the list of messages */
   SMTPResponse(client, PR_DATA, ".");

   /* Report success to caller */
   incrementClientMajorTransaction(client);
   return KWTrue;

} /* commandUIDL */

/*--------------------------------------------------------------------*/
/*       c l e a n u p T r a n a c t i o n                            */
/*                                                                    */
/*       reset variables for a mail command                           */
/*--------------------------------------------------------------------*/

void
cleanupTransaction(SMTPClient *client)
{
   if (client->transaction == NULL)
      return;

   if (client->transaction->mailboxStream != NULL)
   {
      fclose(client->transaction->mailboxStream);
      client->transaction->mailboxStream = NULL;
   }

   if (client->transaction->imf != NULL)
   {
      imclose(client->transaction->imf);
      client->transaction->imf = NULL;
   }

   cleanupMailbox(client->transaction->top);

} /* cleanupTransaction */
