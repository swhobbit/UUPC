/*--------------------------------------------------------------------*/
/*       s m t p r e c v . c                                          */
/*                                                                    */
/*       SMTP commands which actually handle mail                     */
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
 *       $Id: smtprecv.c 1.20 1999/01/08 02:21:01 ahd Exp $
 *
 *       Revision History:
 *       $Log: smtprecv.c $
 *       Revision 1.20  1999/01/08 02:21:01  ahd
 *       Convert currentfile() to RCSID()
 *
 *       Revision 1.19  1999/01/04 03:54:27  ahd
 *       Annual copyright change
 *
 *       Revision 1.18  1998/11/01 20:38:15  ahd
 *       Gracefully handle @@name.com (mangled by remotes from <>)
 *
 *       Revision 1.17  1998/08/02 01:01:27  ahd
 *       Don't bounce inbound bounce messages from mis-configured mailers
 *
 * Revision 1.16  1998/05/11  13:54:34  ahd
 * Correct determination of current local host
 *
 *       Revision 1.15  1998/05/11 01:20:48  ahd
 *       Allow disallowing third-party relaying by default
 *
 *       Revision 1.14  1998/05/08 02:42:15  ahd
 *       Initialize client transaction data structure after allocation
 *       Free client transaction data structure at cleanup
 *
 *       Revision 1.13  1998/04/24 03:30:13  ahd
 *       Use local buffers, not client->transmit.buffer, for output
 *       Rename receive buffer, use pointer into buffer rather than
 *            moving buffered data to front of buffer every line
 *       Restructure main processing loop to give more priority
 *            to client processing data already buffered
 *       Add flag bits to client structure
 *       Add flag bits to verb tables
 *
 *       Revision 1.12  1998/03/16 06:39:32  ahd
 *       Add trumpet remote user support
 *
 *       Revision 1.11  1998/03/08 23:07:12  ahd
 *       Better support of remote vs. local delivery
 *
 *       Revision 1.10  1998/03/06 06:51:28  ahd
 *       Correct false detection of NULL in input
 *
 *       Revision 1.9  1998/03/01 19:43:33  ahd
 *       First compiling POP3 server which accepts user id/password
 *
 *       Revision 1.8  1998/03/01 01:32:25  ahd
 *       Annual Copyright Update
 *
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

RCSID("$Id: smtprecv.c 1.20 1999/01/08 02:21:01 ahd Exp $");

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
   char xmitBuf[XMIT_LENGTH];
   char response[MAXADDR];
   KWBoolean ourProblem;

   if (! stripAddress(operands[0], response))
   {
      sprintf(xmitBuf,
               "%.100s: %s",
               operands[0],
               response);
      SMTPResponse(client, SR_PE_SYNTAX, xmitBuf);
      return KWFalse;
   }

   if (! isValidAddress(operands[0], response, &ourProblem))
   {
      sprintf(xmitBuf,
               "%s: %s",
               operands[0],
               response);
      SMTPResponse(client, SR_PE_BAD_MAILBOX, xmitBuf);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*      We have a valid address, make it canonical and report it      */
/*--------------------------------------------------------------------*/

   sprintf(xmitBuf, "<%s>", operands[0]);
   SMTPResponse(client, verb->successResponse, xmitBuf);
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
   char xmitBuf[XMIT_LENGTH];
   KWBoolean ourProblem;
   size_t lenHost;
   size_t lenDomain;

/*--------------------------------------------------------------------*/
/*   Perform basic examination of address and remove delimiters <>    */
/*--------------------------------------------------------------------*/

   if (! stripAddress(operands[0], response))
   {
      sprintf(xmitBuf,
               "%.100s: %s",
               operands[0],
               response);
      SMTPResponse(client, SR_PE_SYNTAX, xmitBuf);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*       Handle special case of @@nodename, which is what some        */
/*       bogus mailers use for the postmaster by mistake              */
/*--------------------------------------------------------------------*/

   if (strlen(operands[0]) > 2 &&
       equaln(operands[0], "@@", 2) &&
       (strchr(operands[0] + 2, '@') == NULL))
   {
      printmsg(0,"%s: Invalid address %s from %s "
                 "replaced by postmaster address",
                 mName,
                 operands[0],
                 client->connection.hostName );
       strcpy( operands[0], "<>");
   }

/*--------------------------------------------------------------------*/
/*                         Validate the address                       */
/*--------------------------------------------------------------------*/

   if (! isValidAddress(operands[0], response, &ourProblem))
   {
      sprintf(xmitBuf,
               "%s: %s",
               operands[0],
               response);
      SMTPResponse(client, SR_PE_BAD_MAILBOX, xmitBuf);
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
      cleanupTransaction(client);
   }

   client->transaction = malloc(sizeof *(client->transaction));
   checkref(client->transaction);
   memset(client->transaction, 0, sizeof *(client->transaction));

   strcpy( client->transaction->sender, operands[0]);

   client->transaction->addressLength = MAXADDRS;
   client->transaction->addressCount  = 0;
   client->transaction->address =
               malloc(sizeof client->transaction->address[0] *
                       client->transaction->addressLength);
   checkref(client->transaction->address);
   client->transaction->localSender = ourProblem;

/*--------------------------------------------------------------------*/
/*       Check true name of host to see if it is a local relay        */
/*--------------------------------------------------------------------*/

   lenHost = strlen(client->connection.hostName);
   lenDomain  = strlen(E_localdomain);

   if (equal(client->connection.hostName, "localhost") ||
       ((lenDomain <= lenHost) &&
         equal(E_localdomain,
               client->connection.hostName + lenHost - lenDomain)))
   {
      client->transaction->localRelay = KWTrue;
      printmsg(8,"%s: Client %s is a considered local relay.",
                  mName,
                  client->connection.hostName );

   } /* if ((lenDomain >= lenHost) && ... */

/*--------------------------------------------------------------------*/
/*           We're ready for the addressee list, ask for it           */
/*--------------------------------------------------------------------*/

   sprintf(xmitBuf,
           "%s Sender Address okay (%s), specify receiver addresses",
           client->transaction->sender,
           response);

   SMTPResponse(client,
                verb->successResponse,
                xmitBuf );
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
   char xmitBuf[XMIT_LENGTH];
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
      sprintf(xmitBuf,
               "%.100s: %s",
               operands[0],
               response);
      SMTPResponse(client, SR_PE_SYNTAX, xmitBuf);
      return KWFalse;
   }

   if (! isValidAddress(operands[0], response, &ourProblem))
   {
      sprintf(xmitBuf,
               "%s: %s",
               operands[0],
               response);
      SMTPResponse(client, SR_PE_BAD_MAILBOX, xmitBuf);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*      Don't relay for third parties unless explicitly allowed       */
/*--------------------------------------------------------------------*/

   if (!(ourProblem ||
         bflag[F_PROMISCUOUSRELAY] ||
         client->transaction->localRelay))
   {
      sprintf(xmitBuf,
              "%s: Relaying for third-party denied by site policy",
              operands[0] );
      SMTPResponse(client, SR_PE_NOT_POLICY, xmitBuf);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*              The address is good, add it to the list               */
/*--------------------------------------------------------------------*/

   client->transaction->address[ client->transaction->addressCount ] =
                        strdup(operands[0]);
   checkref(client->transaction->address[ client->transaction->addressCount ]);

   sprintf(xmitBuf,
            "<%s>... Okay (%s)",
            client->transaction->address[ client->transaction->addressCount ],
            response);
   client->transaction->addressCount += 1;

   SMTPResponse(client, verb->successResponse, xmitBuf);

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
   char xmitBuf[XMIT_LENGTH];

/*--------------------------------------------------------------------*/
/*              Open our data file, with error checking               */
/*--------------------------------------------------------------------*/

   client->transaction->imf = imopen(0, TEXT_MODE);

   if (client->transaction->imf == NULL)
   {
      sprintf(xmitBuf,
               "Work file IMOpen failed: %s",
               strerror(errno));
      SMTPResponse(client, SR_TE_SHORTAGE, xmitBuf);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*             Generate required "Received" header lines              */
/*--------------------------------------------------------------------*/

   imprintf(client->transaction->imf,
            "%-10s from %s by %s (%s %s) with %sSMTP\n"
                           "%-10s for %s; %s\n",
            "Received:",
            client->clientName,
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
   static const char mName[] = "commandDataInput";
   char xmitBuf[XMIT_LENGTH];
   char *token = client->receive.line;
   char *first = token;
   size_t lineLength = (size_t) client->receive.lineLength;
   size_t stringLength;

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
      lineLength--;
   }

   stringLength = strlen(first);
   if ( stringLength < lineLength)
   {
      SMTPResponse(client,
                   SR_PE_TEMP_SYNTAX,
                   "Data contains Null (0x00) characters");

      printmsg(0,"%s: NULL in data after %d characters, "
                 "total line length %d",
                 mName,
                 strlen( token ),
                 lineLength );
      printmsg(0, "%s Line in error begins: \"%s\"",
                 mName,
                 client->receive.line );

      /* Flush the receipt of data */
      imclose(client->transaction->imf);
      client->transaction->imf = NULL;
      return KWFalse;
   }

#ifdef UDEBUG
   if (strlen(token) > lineLength)
   {
      sprintf(xmitBuf,
               "Internal error: Data line (%d bytes)"
               " longer than expected (%d bytes)",
               strlen(token),
               lineLength);
      SMTPResponse(client, SR_PE_TEMP_SYNTAX, xmitBuf);
      return KWFalse;
   }
#endif

/*--------------------------------------------------------------------*/
/*                          Write the data out                        */
/*--------------------------------------------------------------------*/

   written = imwrite(first, 1, lineLength, client->transaction->imf);

   if (written < (int) strlen(first))
   {
      sprintf(xmitBuf,
               "Work file write failed: %s",
               strerror(errno));
      SMTPResponse(client, SR_TE_SHORTAGE, xmitBuf);
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
   char xmitBuf[XMIT_LENGTH];

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

   bflag[F_FASTSMTP] = KWFalse;  /* Don't attempt outbound SMTP      */

   tokenizeAddress( client->transaction->sender, NULL, fHost, fUser );

   memset( &sender, 0, sizeof sender );
   sender.address = client->transaction->sender;
   sender.relay   = client->clientName;
   sender.host    = fHost;
   sender.user    = fUser;
   sender.remote  = KWTrue;
   sender.daemon  = KWTrue;
   sender.activeUser = "uusmtpd";

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

   sprintf(xmitBuf,
            "Message accepted for delivery to %d mail boxes",
            delivered);

   /* Queue UUXQT to run when client terminates */
   setClientQueueRun( client, KWTrue );

   /* Let client continue on */
   SMTPResponse(client, verb->successResponse, xmitBuf);

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
#ifdef UDEBUG
   memset(client->transaction->address,
                  0,
                  sizeof *(client->transaction->address));
#endif
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
#ifdef UDEBUG
   memset(client->transaction->address,
                  0,
                  sizeof *(client->transaction->address));
#endif
      free(client->transaction->address);
      client->transaction->address = NULL;
   }

#ifdef UDEBUG
   memset(client->transaction, 0, sizeof *(client->transaction));
#endif
   free(client->transaction);
   client->transaction = NULL;

} /* cleanupTransaction */
