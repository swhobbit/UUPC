/*--------------------------------------------------------------------*/
/*       p o p 3 l w c . c                                            */
/*                                                                    */
/*       Light-weight POP3 server verb processors                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by the   */
/*       UUPC/extended license agreement.                             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: pop3lwc.c 1.8 1999/01/04 03:54:27 ahd Exp $
 *
 *       Revision History:
 *       $Log: pop3lwc.c $
 *       Revision 1.8  1999/01/04 03:54:27  ahd
 *       Annual copyright change
 *
 *       Revision 1.7  1998/11/04 01:59:55  ahd
 *       Prevent buffer overflows when processing UIDL lines
 *       in POP3 mail.
 *       Add internal sanity checks for various client structures
 *       Convert various files to CR/LF from LF terminated lines
 *
 *       Revision 1.6  1998/11/01 20:38:15  ahd
 *       Trap overlength operands on USER command
 *
 * Revision 1.5  1998/04/24  03:30:13  ahd
 * Use local buffers, not client->transmit.buffer, for output
 * Rename receive buffer, use pointer into buffer rather than
 *      moving buffered data to front of buffer every line
 * Restructure main processing loop to give more priority
 *      to client processing data already buffered
 * Add flag bits to client structure
 * Add flag bits to verb tables
 *
 *       Revision 1.4  1998/03/08 04:50:04  ahd
 *       Drop unneeded header files
 *
 *       Revision 1.3  1998/03/06 06:51:28  ahd
 *       Add commands to make Netscape happy
 *
 *       Revision 1.2  1998/03/03 03:53:54  ahd
 *       Routines to handle messages within a POP3 mailbox
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

#include "pop3lwc.h"
#include "smtpnetw.h"

#include "timestmp.h"
#include "arpadate.h"
#include "deliver.h"

/*--------------------------------------------------------------------*/
/*                            Global files                            */
/*--------------------------------------------------------------------*/

RCSID("$Id: pop3lwc.c 1.8 1999/01/04 03:54:27 ahd Exp $");

/*--------------------------------------------------------------------*/
/*       c o m m a n d I n i t                                        */
/*                                                                    */
/*       Issue initial POP3 prompt to user                            */
/*--------------------------------------------------------------------*/

KWBoolean
commandInit(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{
   char xmitBuf[XMIT_LENGTH];

   sprintf(xmitBuf,
            "%s POP3 (%s %s, built %s %s) on-line at %s",
            E_domain,
            compilep,
            compilev,
            compiled,
            compilet,
            arpadate());

   SMTPResponse(client, verb->successResponse, xmitBuf);

   setClientTimeout( client, 600 );

   return KWTrue;
}

/*--------------------------------------------------------------------*/
/*       c o m m a n d U S E R                                        */
/*                                                                    */
/*       Accept USER name from POP client                             */
/*--------------------------------------------------------------------*/

KWBoolean
commandUSER(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{
   char xmitBuf[XMIT_LENGTH];

   if ((strlen(operands[0]) + strlen(E_domain)) > MAXADDR)
   {
      SMTPResponse(client, PR_ERROR_GENERIC, "User id is too long" );
      return KWFalse;
   }

   /* Save off the client name, PASS command needs it */
   client->clientName = strdup( operands[0] );
   checkref( client->clientName );

   sprintf( xmitBuf,
            "User %s accepted, send password",
            client->clientName );

   SMTPResponse(client, verb->successResponse, xmitBuf);
   return KWTrue;

} /* commandUSER */

/*--------------------------------------------------------------------*/
/*       r e j e c t L o g i n                                        */
/*                                                                    */
/*       Common processing for rejecting login of a client            */
/*--------------------------------------------------------------------*/

static void
rejectLogin( SMTPClient *client )
{
   static const char FailureMessage[] =
            "Authorization failed, "
            "bad user id, "
            "password, "
            "or shell for POP3 access";

   free( client->clientName );
   client->clientName = NULL;
   SMTPResponse(client, PR_ERROR_GENERIC, FailureMessage );
   setClientMode( client, P3_AUTHORIZATION );

} /* rejectLogin */

/*--------------------------------------------------------------------*/
/*       c o m m a n d P A S S                                        */
/*                                                                    */
/*       Accept password from user and validate password/user         */
/*       combination                                                  */
/*--------------------------------------------------------------------*/

KWBoolean
commandPASS(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{

   struct UserTable *userp;

/*--------------------------------------------------------------------*/
/*                        Validate the user id                        */
/*--------------------------------------------------------------------*/

   userp  = checkuser(client->clientName);
                                    /* Locate user id in host table  */

   if ( userp == BADUSER )
   {
      printmsg(0, "Login rejected for %s, unknown user id",
                  client->clientName );
      rejectLogin( client );
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*                       Validate the password                        */
/*--------------------------------------------------------------------*/

   if ( ! userp->password || !equal(operands[0], userp->password ))
   {
      printmsg(0, "Login rejected for %s, invalid password",
                  client->clientName);
      rejectLogin( client );
      return KWFalse;
   }

   /* Remove password copy from buffer memory */
   memset( operands[0], 'Z', strlen( operands[0] ));

/*--------------------------------------------------------------------*/
/*                         Validate the shell                         */
/*--------------------------------------------------------------------*/

   if ( ! userp->group || !equal(POP3_GROUP, userp->group ))
   {
      printmsg(0, "Login rejected for %s, group is not %s",
                  client->clientName,
                  POP3_GROUP);
      rejectLogin( client );
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*       The user is a valid mailbox and authorized, save his/her     */
/*       information.  We do not issue the user response as this      */
/*       time because the the mailbox is not actually open yet.       */
/*       Rather, our successful return will cause a state change      */
/*       which will cause the mailbox open.                           */
/*--------------------------------------------------------------------*/

   client->transaction = malloc( sizeof *client->transaction );
   checkref( client->transaction );
   memset(client->transaction, 0, sizeof *client->transaction);
   client->transaction->userp = userp;
   client->magic = POP3T_MAGIC;

   /* Immediately process the load of the mailbox */
   setClientProcess(client, KWTrue );

   return KWTrue;

} /* commandPASS */

/*--------------------------------------------------------------------*/
/*       c o m m a n d S e q u e n c e I g n o r e                    */
/*                                                                    */
/*       Handle commands issued out of sequence                       */
/*--------------------------------------------------------------------*/

KWBoolean
commandSequenceIgnore(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{
   /* Success is a loose description here */
   SMTPResponse(client,
                verb->modeErrorResponse,
                "Command issued out of sequence" );

   return KWFalse;

}  /* commandSequenceIgnore */

/*--------------------------------------------------------------------*/
/*       c o m m a n d X S E N D E R                                  */
/*                                                                    */
/*       Dummy command to report (no) authorized sender of message    */
/*--------------------------------------------------------------------*/

KWBoolean
commandXSENDER(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{

/*--------------------------------------------------------------------*/
/*       Success is a loose description here, empty string keeps      */
/*       Netscape happy.                                              */
/*--------------------------------------------------------------------*/

   SMTPResponse(client,
                verb->successResponse,
                "" );

   return KWFalse;

}  /* commandXSENDER */

/*--------------------------------------------------------------------*/
/*       c o m m a n d A U T H                                        */
/*                                                                    */
/*       Report to client we don't support AUTH command               */
/*--------------------------------------------------------------------*/

KWBoolean
commandAUTH(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{
   char xmitBuf[XMIT_LENGTH];

   if ( operands[0] == NULL )
      SMTPResponse(client,
                   PR_ERROR_WARNING,
                   "AUTH capabilities listing not available" );
   else {
      sprintf( xmitBuf,
               "Cannot authenticate with %s method",
               operands[0] );
      SMTPResponse(client,
                   PR_ERROR_GENERIC,
                   xmitBuf );
   }

   return KWFalse;

}  /* commandXSENDER */

/*--------------------------------------------------------------------*/
/*       s e t D e l i v e r y G r a d e                              */
/*                                                                    */
/*       Set the delivery grade for UUCP mail                         */
/*--------------------------------------------------------------------*/

void
setDeliveryGrade( const char inGrade )
{
   /* No operation in POP3 server */
}
