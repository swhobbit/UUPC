/*--------------------------------------------------------------------*/
/*       s m t p r e c v . c                                          */
/*                                                                    */
/*       SMTP commands which actually handle mail                     */
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
 *       $Id: smtprecv.c 1.7 1997/12/14 02:41:14 ahd v1-12u $
 *
 *       Revision History:
 *       $Log: smtprecv.c $
 *       Revision 1.7  1997/12/14 02:41:14  ahd
 *       Change allocation of SMTP sender address
 *
 *       Revision 1.6  1997/12/13 18:05:06  ahd
 *       Change parsing and passing of sender address information
 *
 *       Revision 1.5  1997/11/29 13:03:13  ahd
 *       Clean up single client (hot handle) mode for OS/2, including correct
 *       network initialization, use unique client id (pid), and invoke all
 *       routines needed in main client loop.
 *
 *       Revision 1.4  1997/11/28 23:11:38  ahd
 *       Additional SMTP auditing, normalize formatting, more OS/2 SMTP fixes
 *
 *       Revision 1.3  1997/11/26 03:34:11  ahd
 *       Correct SMTP timeouts, break out protocol from rest of daemon
 *
 *       Revision 1.2  1997/11/24 02:52:26  ahd
 *       First working SMTP daemon which delivers mail
 *
 *       Revision 1.1  1997/11/21 18:15:18  ahd
 *       Command processing stub SMTP daemon
 *
 */

/*--------------------------------------------------------------------*/
/*                           include files                            */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include "timestmp.h"
#include "smtprecv.h"
#include "smtpnetw.h"
#include "smtputil.h"
#include "arpadate.h"
#include "address.h"
#include "deliver.h"
#include "mail.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtprecv.c 1.7 1997/12/14 02:41:14 ahd v1-12u $");

currentfile();

/*--------------------------------------------------------------------*/
/*       c o m m a n d V R F Y                                        */
/*                                                                    */
/*       Verify syntax and location of address; does not save or      */
/*       otherwise process addresss                                   */
/*--------------------------------------------------------------------*/

KWBoolean
commandVRFY(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{
   char response[MAXADDR];
   KWBoolean ourProblem;

   if (! stripAddress(operands[0], response))
   {
      sprintf(client->transmit.data,
               "%.100s: %s",
               operands[0],
               response);
      SMTPResponse(client, SR_PE_SYNTAX, client->transmit.data);
      return KWFalse;
   }

   if (! isValidAddress(operands[0], response, &ourProblem))
   {
      sprintf(client->transmit.data,
               "%s: %s",
               operands[0],
               response);
      SMTPResponse(client, SR_PE_BAD_MAILBOX, client->transmit.data);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*      We have a valid address, make it canonical and report it      */
/*--------------------------------------------------------------------*/

   sprintf(client->transmit.data, "<%s>", operands[0]);
   SMTPResponse(client, verb->successResponse, client->transmit.data);
   return KWTrue;

} /* commandVRFY */

/*--------------------------------------------------------------------*/
/*       c o m m a n d M A I L                                        */
/*                                                                    */
/*       Initiate a new SMTP mail transaction, including recording    */
/*       the sender address                                           */
/*--------------------------------------------------------------------*/

KWBoolean
commandMAIL(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{
   static const char mName[] = "commandMAIL";
   char response[MAXADDR];
   KWBoolean ourProblem;

   if (! stripAddress(operands[0], response))
   {
      sprintf(client->transmit.data,
               "%.100s: %s",
               operands[0],
               response);
      SMTPResponse(client, SR_PE_SYNTAX, client->transmit.data);
      return KWFalse;
   }

   if (! isValidAddress(operands[0], response, &ourProblem))
   {
      sprintf(client->transmit.data,
               "%s: %s",
               operands[0],
               response);
      SMTPResponse(client, SR_PE_BAD_MAILBOX, client->transmit.data);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*       Information is valid, save sender and allocate receiver      */
/*       array                                                        */
/*--------------------------------------------------------------------*/

   if (client->transaction != NULL)
   {
      printmsg(0,"%s: Internal error, client %d transaction already allocated",
                 mName,
                 getClientSequence(client));
   }

   client->transaction = malloc(sizeof *(client->transaction));
   checkref(client->transaction);

   strcpy( client->transaction->sender, operands[0]);

   client->transaction->addressLength = MAXADDRS;
   client->transaction->addressCount  = 0;
   client->transaction->address =
               malloc(sizeof client->transaction->address[0] *
                       client->transaction->addressLength);
   checkref(client->transaction->address);

/*--------------------------------------------------------------------*/
/*           We're ready for the addressee list, ask for it           */
/*--------------------------------------------------------------------*/

   sprintf(client->transmit.data,
           "%s Sender Address okay, specify receiver addresses",
           client->transaction->sender);
   SMTPResponse(client,
                 verb->successResponse,
                 client->transmit.data );
   return KWTrue;

} /* commandMAIL */

/*--------------------------------------------------------------------*/
/*       c o m m a n d R C P T                                        */
/*                                                                    */
/*       Process one receiver address                                 */
/*--------------------------------------------------------------------*/

KWBoolean
commandRCPT(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{
   char response[MAXADDR];
   KWBoolean ourProblem;

   if (client->transaction->addressCount >=
        client->transaction->addressLength)
   {
      SMTPResponse(client,
                    SR_PE_TOO_MANY_ADDR,
                    "Cannot handle additional addresses in message");
      return KWFalse;
   }

   if (! stripAddress(operands[0], response))
   {
      sprintf(client->transmit.data,
               "%.100s: %s",
               operands[0],
               response);
      SMTPResponse(client, SR_PE_SYNTAX, client->transmit.data);
      return KWFalse;
   }

   if (! isValidAddress(operands[0], response, &ourProblem))
   {
      sprintf(client->transmit.data,
               "%s: %s",
               operands[0],
               response);
      SMTPResponse(client, SR_PE_BAD_MAILBOX, client->transmit.data);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*              The address is good, add it to the list               */
/*--------------------------------------------------------------------*/

   client->transaction->address[ client->transaction->addressCount ] =
                        strdup(operands[0]);
   checkref(client->transaction->address[ client->transaction->addressCount ]);

   sprintf(client->transmit.data,
            "<%s>... Okay (%s)",
            client->transaction->address[ client->transaction->addressCount ],
            response);
   client->transaction->addressCount += 1;

   SMTPResponse(client, verb->successResponse, client->transmit.data);

   return KWTrue;

} /* commandRCPT */

/*--------------------------------------------------------------------*/
/*       c o m m a n d D A T A                                        */
/*                                                                    */
/*       Begin processing data for mail                               */
/*--------------------------------------------------------------------*/

KWBoolean
commandDATA(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{
   char *forwho = (char *) ((client->transaction->addressCount > 1) ?
                              "multiple addresses" :
                              client->transaction->address[0]);

/*--------------------------------------------------------------------*/
/*              Open our data file, with error checking               */
/*--------------------------------------------------------------------*/

   client->transaction->imf = imopen(0, TEXT_MODE);

   if (client->transaction->imf == NULL)
   {
      sprintf(client->transmit.data,
               "Work file IMOpen failed: %s",
               strerror(errno));
      SMTPResponse(client, SR_TE_SHORTAGE, client->transmit.data);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*             Generate required "Received" header lines              */
/*--------------------------------------------------------------------*/

   imprintf(client->transaction->imf,
            "%-10s from %s by %s (%s %s) with %sSMTP\n"
                           "%-10s for %s; %s\n",
            "Received:",
            client->SMTPName,
            E_domain,
            compilep,
            compilev,
            client->esmtp ? "E" : "",
            " ",
            forwho,
            arpadate());

/*--------------------------------------------------------------------*/
/*            We're ready to process the data, tell client            */
/*--------------------------------------------------------------------*/

   SMTPResponse(client,
                 verb->successResponse,
                "Enter mail, end with \".\" on a line by itself");

   return KWTrue;

} /* commandDATA */

/*--------------------------------------------------------------------*/
/*       c o m m a n d D a t a I n p u t                              */
/*                                                                    */
/*       Accept more data for email processing                        */
/*--------------------------------------------------------------------*/

KWBoolean
commandDataInput(SMTPClient *client,
                 struct _SMTPVerb* verb,
                 char **operands)
{
   char *token = client->receive.data;
   char *first = token;
   size_t len = (size_t) client->receive.parsed - 2;
   int written;

   if (client->transaction->imf == NULL)     /* Previously flushed?  */
      return KWFalse;               /* Yes --> Ignore data           */

/*--------------------------------------------------------------------*/
/*       Determine if the entire buffer is composed of periods        */
/*--------------------------------------------------------------------*/

   while(*token == '.')             /* Scan until EOS or non period  */
      token++;

   if (token == '\0')               /* Was it EOS?                   */
   {
      first++;                      /* Yes --> Skip quoting period   */
      len--;
   }

   if (strlen(token) < len)
   {
      SMTPResponse(client,
                    SR_PE_SYNTAX,
                    "Data contains Null (0x00) characters");

      /* Flush the receipt of data */
      imclose(client->transaction->imf);
      client->transaction->imf = NULL;
      return KWFalse;
   }

#ifdef UDEBUG
   if (strlen(token) > len)
   {
      sprintf(client->transmit.data,
               "Internal error: Data line (%d bytes)"
               " longer than expected (%d bytes)",
               strlen(token),
               len);
      SMTPResponse(client, SR_PE_SYNTAX, client->transmit.data);
      return KWFalse;
   }
#endif

/*--------------------------------------------------------------------*/
/*                          Write the data out                        */
/*--------------------------------------------------------------------*/

   written = imwrite(first, 1, len, client->transaction->imf);

   if (written < (int) strlen(first))
   {
      sprintf(client->transmit.data,
               "Work file write failed: %s",
               strerror(errno));
      SMTPResponse(client, SR_TE_SHORTAGE, client->transmit.data);
      imclose(client->transaction->imf);
      client->transaction->imf = NULL;
      return KWFalse;
   }

   imputc('\n', client->transaction->imf);

   return KWTrue;

} /* commandDataInput */

/*--------------------------------------------------------------------*/
/*       c o m m a n d P e r i o d                                    */
/*                                                                    */
/*       Process end of data stream by sending mail off for delivery  */
/*--------------------------------------------------------------------*/

KWBoolean
commandPeriod(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{

   size_t count;
   size_t delivered = 0;
   MAIL_ADDR sender;
   char fUser[MAXADDR];
   char fHost[MAXADDR];

/*--------------------------------------------------------------------*/
/*       If we previously had problem and issues error reply, just    */
/*       cleanup now and allow a return to idle state without an      */
/*       additional message.                                          */
/*--------------------------------------------------------------------*/

   if (client->transaction->imf == NULL)     /* Already flushed?     */
   {
      cleanupTransaction(client); /* Handle any dangling states      */
      return KWTrue;                /* Allow return to idle state    */
   }

/*--------------------------------------------------------------------*/
/*       And now, ladies and gentlemen, boys and girls, the moment    */
/*       we have ALL been waiting for ... delivery of the message.    */
/*--------------------------------------------------------------------*/

   bflag[F_FASTSMTP] = KWFalse;  /* Don't do outbound SMTP           */

   tokenizeAddress( client->transaction->sender, NULL, fHost, fUser );

   memset( &sender, 0, sizeof sender );
   sender.address = client->transaction->sender;
   sender.relay   = client->SMTPName;
   sender.host    = fHost;
   sender.user    = fUser;

   for (count = 0; count < client->transaction->addressCount; count++)
      delivered += Deliver(client->transaction->imf,
                           &sender,
                           client->transaction->address[count],
                           KWTrue);

   flushQueues(client->transaction->imf,
               &sender );

   cleanupTransaction(client);

/*--------------------------------------------------------------------*/
/*               Tell the client we delivered the message             */
/*--------------------------------------------------------------------*/

   sprintf(client->transmit.data,
            "Message accepted for delivery to %d mail boxes",
            delivered);

   SMTPResponse(client, verb->successResponse, client->transmit.data);
                                    /* Let client continue on        */

   incrementClientMajorTransaction( client );
   return KWTrue;

} /* commandPeriod */

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

   if (client->transaction->imf)
   {
      imclose(client->transaction->imf);
      client->transaction->imf = NULL;
   }

   if (client->transaction->addressCount)
   {
      size_t count;
      for (count = 0;
            count < client->transaction->addressCount;
            count ++)
      {
         free(client->transaction->address[ count ]);
         client->transaction->address[ count ] = NULL;
      }

      client->transaction->addressCount = 0;
   }

   if (client->transaction->address)
   {
      free(client->transaction->address);
      client->transaction->address = NULL;
   }

   client->transaction = NULL;

} /* cleanupTransaction */
