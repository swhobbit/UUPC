/*--------------------------------------------------------------------*/
/*       p o p 3 u s e r . c                                          */
/*                                                                    */
/*       POP3 specific heavy processes                                */
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
 *       $Id: pop3user.c 1.1 1998/03/01 19:42:17 ahd Exp $
 *
 *       Revision History:
 *       $Log: pop3user.c $
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

RCSID("$Id: pop3user.c 1.1 1998/03/01 19:42:17 ahd Exp $");

currentfile();

/*--------------------------------------------------------------------*/
/*       g e t P o p M e s s a g e                                    */
/*                                                                    */
/*       Get pointer to requested POP message                         */
/*--------------------------------------------------------------------*/

static MailMessage *
getPopMessage(SMTPClient *client, const char *number)
{
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

   sprintf(client->transmit.data,
            "Cannot retrieve message %s of %ld for user %s, error: %s",
            number,
            client->transaction->messageCount,
            client->clientName,
            errorText);
   SMTPResponse(client, PR_ERROR_GENERIC, client->transmit.data);

   return NULL;

} /* getPopMessage */

/*--------------------------------------------------------------------*/
/*       i s A l l P e r i o d s                                      */
/*                                                                    */
/*       Determine if a character array of specified length is all    */
/*       periods.                                                     */
/*--------------------------------------------------------------------*/

static KWBoolean
isAllPeriods(char *s, int len)      /* COPIED -- FIX ME! */
{
   int column;
   for (column = 0; column < len; column++)
   {
      if (s[column] != '.')
         return KWFalse;
   }

   return KWTrue;

} /* allPeriods */

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
   if (imseek(client->transaction->imf,
               current->startPosition,
               SEEK_SET) == -1)
   {
      printerr("imfile");
      SMTPResponse(client, PR_ERROR_GENERIC, "Seek failed");
      return;
   }

   SMTPResponse(client, PR_OK_GENERIC, "Requested Message follows");

   for (;;)
   {
      long position = imtell(client->transaction->imf);
      size_t length;
      char buffer[BUFSIZ];

      if (position >= current->endPosition)
         break;

      if ((position >= current->startBodyPosition) && (bodyLines-- < 0))
         break;

      if (imgets(buffer,
                 sizeof buffer - 2,
                 client->transaction->imf) == NULL)
         break;

      length = strlen(buffer);

      if (buffer[length - 1] == '\n')
         buffer[--length] = '\0';
      else {
         printmsg(0,"Cannot process over length buffer for tranmission");
         panic();                       /* FIX ME */
      }

      if ((length > 0) && isAllPeriods(buffer, length))
         strcat(buffer, ".");

      SMTPResponse(client, PR_DATA, buffer);

   } /* for (;;) */

   /* Terminate the message */
   SMTPResponse(client, PR_DATA, ".");

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

/*--------------------------------------------------------------------*/
/*                   Create the final mailbox name                    */
/*--------------------------------------------------------------------*/

   strncpy(client->transaction->mailboxName,
            client->clientName,
            sizeof client->transaction->mailboxName);

   client->transaction->mailboxName[ sizeof
            client->transaction->mailboxName - 1 ] = '\0';

   if (expand_path(client->transaction->mailboxName,
                    client->transaction->userp->homedir,
                    client->transaction->userp->homedir,
                    E_mailext) == NULL)
   {
      sprintf(client->transmit.data,
              "Internal error, cannot determine mailbox location for %s",
              client->clientName);

      SMTPResponse(client, PR_ERROR_GENERIC, client->transmit.data);
      setClientMode(client, P3_AUTHORIZATION);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*                Get any new mail from system mailbox                */
/*--------------------------------------------------------------------*/

   if (! IncludeNew(client->transaction->mailboxName,
                      client->clientName))
   {
      sprintf(client->transmit.data,
              "Internal error, unable to append new mail to mailbox %s: %s",
              client->transaction->mailboxName,
              strerror(errno));
      SMTPResponse(client, PR_ERROR_GENERIC, client->transmit.data);
      setClientMode(client, P3_AUTHORIZATION);
      return KWFalse;
   }


/*--------------------------------------------------------------------*/
/*         Load the mailbox information, with error checking          */
/*--------------------------------------------------------------------*/

   if (! popBoxLoad(client))
   {

      sprintf(client->transmit.data,
              "Internal error, unable to load mailbox %s",
              client->transaction->mailboxName);

      SMTPResponse(client, PR_ERROR_GENERIC, client->transmit.data);
      setClientMode(client, P3_AUTHORIZATION);
      return KWFalse;

   } /* if (! popBoxLoad(client)) */

/*--------------------------------------------------------------------*/
/*                     Report our mailbox is open                     */
/*--------------------------------------------------------------------*/

   sprintf(client->transmit.data,
           "%s has %ld message(s) (%ld octets)",
           client->clientName,
           client->transaction->messageCount,
           getMessageOctetCount(client->transaction->top, NULL));

   SMTPResponse(client,
                verb->successResponse,
                client->transmit.data);

   return KWTrue;

} /* commandLoadMailbox */

KWBoolean
commandDataOutput(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{
   return KWTrue;
}

KWBoolean
commandDELE(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{

   MailMessage *current = getPopMessage(client, operands[0]);

   if (current == NULL)
      return KWFalse;

   popBoxDelete(current);

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

   sprintf(client->transmit.data, "%ld %ld",
           current->sequence,
           current->octets);
   SMTPResponse(client, code, client->transmit.data);

} /* listOneMessage */

KWBoolean
commandLIST(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{
   MailMessage *current;
   long octets;
   long messages;

/*--------------------------------------------------------------------*/
/*                Handle a single message to report on                */
/*--------------------------------------------------------------------*/

   if (operands[0] != NULL)
   {
      current = getPopMessage(client, operands[0]);
      if (current == NULL)
         return KWFalse;
      listOneMessage(client, current, verb->successResponse);
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*                  Report on all undeleted messages                  */
/*--------------------------------------------------------------------*/

   /* Send the header line */
   octets = getMessageOctetCount(client->transaction->top, &messages);

   sprintf(client->transmit.data,
           "%ld message(s) (%ld octets)",
           messages, octets);
   SMTPResponse(client, verb->successResponse, client->transmit.data);

   /* Now loop through the actual messages to report on */
   current = client->transaction->top;
   while(current != NULL)
   {
      listOneMessage(client, current, PR_DATA);
      current = getPopMessageNext(current);
   } /* while(current != NULL) */

   /* Terminate the list of messages */
   SMTPResponse(client, PR_DATA, ".");

   /* Report success to caller */
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
   sprintf(client->transmit.data,
            "%s Closing connection, adios",
            E_domain);
   SMTPResponse(client, verb->successResponse, client->transmit.data);
   return KWTrue;
}

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
   MailMessage *current = getPopMessage(client, operands[0]);

   if (current == NULL)
      return KWFalse;

   writePopMessage( client, current, LONG_MAX );

   /* Don't kick into data output mode! */
   return KWFalse;

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

   octets = getMessageOctetCount(client->transaction->top, &messages);

   sprintf(client->transmit.data,
           "%ld %ld",
           messages, octets);
   SMTPResponse(client, verb->successResponse, client->transmit.data);

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
   MailMessage *current = getPopMessage(client, operands[0]);

   if (current == NULL)
      return KWFalse;

   writePopMessage( client, current, atol(operands[1]));

   /* Don't kick into data output mode! */
   return KWFalse;

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

   sprintf(client->transmit.data, "%ld %s",
           current->sequence,
           popBoxUIDL(current));
   SMTPResponse(client, code, client->transmit.data);

} /* listOneMessage */

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
      current = getPopMessage(client, operands[0]);
      if (current == NULL)
         return KWFalse;
      identifyOneMessage(client, current, verb->successResponse);
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
      current = getPopMessageNext(current);
   } /* while(current != NULL) */

   /* Terminate the list of messages */
   SMTPResponse(client, PR_DATA, ".");

   /* Report success to caller */
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