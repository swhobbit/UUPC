/*--------------------------------------------------------------------*/
/*       s m t p r e c v . c                                          */
/*                                                                    */
/*       SMTP commands which actually handle mail                     */
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
 *       $Id: smtprecv.c 1.1 1997/11/21 18:15:18 ahd Exp $
 *
 *       Revision History:
 *       $Log: smtprecv.c $
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

RCSID("$Id: smtprecv.c 1.1 1997/11/21 18:15:18 ahd Exp $");

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
            char **operands )
{
   char response[MAXADDR];
   KWBoolean ourProblem;

   if ( ! stripAddress( operands[0], response ))
   {
      sprintf( client->transmit.data,
               "%.100s: %s",
               operands[0],
               response );
      SMTPResponse( client, SR_PE_SYNTAX, client->transmit.data);
      return KWFalse;
   }

   if ( ! isValidAddress( operands[0], response, &ourProblem ))
   {
      sprintf( client->transmit.data,
               "%s: %s",
               operands[0],
               response );
      SMTPResponse( client, SR_PE_BAD_MAILBOX, client->transmit.data);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*      We have a valid address, make it canonical and report it      */
/*--------------------------------------------------------------------*/

   sprintf( client->transmit.data, "<%s>", operands[0] );
   SMTPResponse( client, verb->successResponse, client->transmit.data );
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
            char **operands )
{
   char response[MAXADDR];
   KWBoolean ourProblem;

   if ( ! stripAddress( operands[0], response ))
   {
      sprintf( client->transmit.data,
               "%.100s: %s",
               operands[0],
               response );
      SMTPResponse( client, SR_PE_SYNTAX, client->transmit.data);
      return KWFalse;
   }

   if ( ! isValidAddress( operands[0], response, &ourProblem ))
   {
      sprintf( client->transmit.data,
               "%s: %s",
               operands[0],
               response );
      SMTPResponse( client, SR_PE_BAD_MAILBOX, client->transmit.data);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*       Information is valid, save sender and allocate receiver      */
/*       array                                                        */
/*--------------------------------------------------------------------*/

   client->sender = strdup( operands[0] );
   checkref( client->sender );

   client->addressLength = MAXADDRS;
   client->addressCount  = 0;
   client->address = malloc( sizeof client->address[0] *
                             client->addressLength );
   checkref( client->address );

/*--------------------------------------------------------------------*/
/*           We're ready for the addressee list, ask for it           */
/*--------------------------------------------------------------------*/

   SMTPResponse( client,
                 verb->successResponse,
                 "Okay, send receiver addresses");
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
            char **operands )
{
   char response[MAXADDR];
   KWBoolean ourProblem;

   if ( client->addressCount >= client->addressLength )
   {
      SMTPResponse( client,
                    SR_PE_TOO_MANY_ADDR,
                    "We cannot handle additional addresses in one message");
      return KWFalse;
   }

   if ( ! stripAddress( operands[0], response ))
   {
      sprintf( client->transmit.data,
               "%.100s: %s",
               operands[0],
               response );
      SMTPResponse( client, SR_PE_SYNTAX, client->transmit.data);
      return KWFalse;
   }

   if ( ! isValidAddress( operands[0], response, &ourProblem ))
   {
      sprintf( client->transmit.data,
               "%s: %s",
               operands[0],
               response );
      SMTPResponse( client, SR_PE_BAD_MAILBOX, client->transmit.data);
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*              The address is good, add it to the list               */
/*--------------------------------------------------------------------*/

   client->address[ client->addressCount ] = strdup( operands[0] );
   checkref( client->address[ client->addressCount ] );

   sprintf( client->transmit.data,
            "<%s>... Okay (%s)",
            client->address[ client->addressCount ],
            response );
   client->addressCount += 1;

   SMTPResponse( client, verb->successResponse, client->transmit.data );

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
            char **operands )
{
   char *forwho = (char *) ((client->addressCount > 1 ) ?
                              "multiple addresses" :
                              client->address[0]);

/*--------------------------------------------------------------------*/
/*              Open our data file, with error checking               */
/*--------------------------------------------------------------------*/

   client->imf = imopen( 0, TEXT_MODE );

   if ( client->imf == NULL )
   {
      sprintf( client->transmit.data,
               "Work file IMOpen failed: %s",
               strerror(errno) );
      SMTPResponse( client, SR_TE_SHORTAGE, client->transmit.data );
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*             Generate required "Received" header lines              */
/*--------------------------------------------------------------------*/

   imprintf(client->imf,"%-10s from %s by %s (%s %s) with %sSMTP\n"
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

   SMTPResponse( client,
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
                 char **operands )
{
   char *token = client->receive.data;
   char *first = token;
   size_t len = (size_t) client->receive.parsed - 2;
   int written;

   if ( client->imf == NULL )       /* Previously flushed?           */
      return KWFalse;               /* Yes --> Ignore data           */

/*--------------------------------------------------------------------*/
/*       Determine if the entire buffer is composed of periods        */
/*--------------------------------------------------------------------*/

   while( *token == '.' )           /* Scan until EOS or non period  */
      token++;

   if ( token == '\0')              /* Was it EOS?                   */
   {
      first++;                      /* Yes --> Skip quoting period   */
      len--;
   }

   if ( strlen( token ) < len )
   {
      SMTPResponse( client,
                    SR_PE_SYNTAX,
                    "Data contains Null (0x00) characters" );

      /* Flush the receipt of data */
      imclose( client->imf );
      client->imf = NULL;
      return KWFalse;
   }

#ifdef UDEBUG
   if ( strlen( token ) > len )
   {
      sprintf( client->transmit.data,
               "Internal error: Data line (%d bytes)"
               " longer than expected (%d bytes)",
               strlen( token ),
               len );
      SMTPResponse( client, SR_PE_SYNTAX, client->transmit.data );
      return KWFalse;
   }
#endif

/*--------------------------------------------------------------------*/
/*                          Write the data out                        */
/*--------------------------------------------------------------------*/

   written = imwrite( first, 1, len, client->imf );

   if ( written < (int) strlen( first ))
   {
      sprintf( client->transmit.data,
               "Work file write failed: %s",
               strerror(errno) );
      SMTPResponse( client, SR_TE_SHORTAGE, client->transmit.data );
      imclose( client->imf );
      client->imf = NULL;
      return KWFalse;
   }

   imputc( '\n', client->imf );

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
            char **operands )
{

   size_t count;
   size_t delivered = 0;

/*--------------------------------------------------------------------*/
/*       If we previously had problem and issues error reply, just    */
/*       cleanup now and allow a return to idle state without an      */
/*       additional message.                                          */
/*--------------------------------------------------------------------*/

   if ( client->imf == NULL )       /* Stream previously flushed?    */
   {
      cleanupClientMail( client );  /* Handle any dangling states    */
      return KWTrue;                /* Allow return to idle state    */
   }

/*--------------------------------------------------------------------*/
/*             Set flags up for the internal delivery engine          */
/*--------------------------------------------------------------------*/

   bflag[F_FASTSMTP] = KWFalse;
   remoteMail = KWTrue;

   tokenizeAddress( client->sender,
                    client->transmit.data,
                    fromNode,
                    fromUser);      /* No need to check return code,
                                       was validated when accepted   */

   rnode = fromNode;
   ruser = fromUser;
   uuser = "uucp";
   grade = E_mailGrade;          /* Get grade from configuration     */

/*--------------------------------------------------------------------*/
/*       And now, ladies and gentlemen, boys and girls, the moment    */
/*       we have ALL been waiting for ... delivery of the message.    */
/*--------------------------------------------------------------------*/

   bflag[F_FASTSMTP] = KWFalse;  /* Don't do outbound SMTP           */
   remoteMail = KWTrue;          /* Mail did not originate here      */

   for ( count = 0; count < client->addressCount; count++)
      delivered += Deliver(client->imf, client->address[count], KWTrue);

   flushQueues( client->imf );

   cleanupClientMail( client );

/*--------------------------------------------------------------------*/
/*               Tell the client we delivered the message             */
/*--------------------------------------------------------------------*/

   sprintf( client->transmit.data,
            "Message accepted for delivery to %d mail boxes",
            delivered );

   SMTPResponse( client, verb->successResponse, client->transmit.data );
                                    /* Let client continue on        */

   return KWTrue;

} /* commandPeriod */
