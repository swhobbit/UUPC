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

#include "pop3lwc.h"
#include "smtpnetw.h"

#include "timestmp.h"
#include "arpadate.h"
#include "deliver.h"

/*--------------------------------------------------------------------*/
/*                            Global files                            */
/*--------------------------------------------------------------------*/

RCSID("$Id: pop3lwc.c 1.1 1998/03/01 19:42:17 ahd Exp $");

currentfile();

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

   sprintf(client->transmit.data,
            "%s POP3 (%s %s, built %s %s) on-line at %s",
            E_domain,
            compilep,
            compilev,
            compiled,
            compilet,
            arpadate());

   SMTPResponse(client, verb->successResponse, client->transmit.data);

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

   /* Save off the client name, PASS command needs it */
   client->clientName = strdup( operands[0] );
   checkref( client->clientName );

   sprintf( client->transmit.data,
            "User %s accepted, send password",
            client->clientName );

   SMTPResponse(client, verb->successResponse, client->transmit.data);
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
/*       s e t D e l i v e r y G r a d e                              */
/*                                                                    */
/*       Set the delivery grade for UUCP mail                         */
/*--------------------------------------------------------------------*/

void
setDeliveryGrade( const char inGrade )
{
   /* No operation in POP3 server */
}
