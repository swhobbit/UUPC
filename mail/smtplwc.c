/*--------------------------------------------------------------------*/
/*       s m t p l w c . c                                            */
/*                                                                    */
/*       Light-weight SMTP server verb processors                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1997 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by the   */
/*       UUPC/extended license agreement.                             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: smtpverb.c 1.1 1997/06/03 03:25:31 ahd Exp $
 *
 *       Revision History:
 *       $Log$
 */

#include "uupcmoah.h"
#include "smtplwc.h"
#include "smtprecv.h"
#include "smtpnetw.h"
#include "timestmp.h"

RCSID("$Id: smtpclnt.c 1.1 1997/06/03 03:25:31 ahd Exp $");

currentfile();

/*--------------------------------------------------------------------*/
/*       c o m m a n d I n i t                                        */
/*                                                                    */
/*       Perform initial command processing (greeting)                */
/*--------------------------------------------------------------------*/

commandInit(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{

/*--------------------------------------------------------------------*/
/*       This message encourages E-SMTP processing (EHLO command),    */
/*       as while we don't support any special SMTP extensions, we    */
/*       can say that in the EHLO command.                            */
/*--------------------------------------------------------------------*/

   sprintf( client->transmit.data,
            "%s ESMTP (%s %s, built %s %s)",
            E_domain,
            compilep,
            compilev,
            compiled,
            compilet );

   SMTPResponse( client, SR_OK_CONNECT, client->transmit.data );

   return KWTrue;
}

/*--------------------------------------------------------------------*/
/*       c o m m a n d  H E L O                                       */
/*                                                                    */
/*       Respond to client greeting                                   */
/*--------------------------------------------------------------------*/

commandHELO(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{

/*--------------------------------------------------------------------*/
/*                 Save the name the client claims to be              */
/*                                                                    */
/*       We could check for a domain name (at least one period in     */
/*       the name), but we don't at this point.                       */
/*--------------------------------------------------------------------*/

   client->SMTPName = strdup( operands[0] );
   checkref( client->SMTPName );
   if ( strlen( client->SMTPName ) > MAXADDR )
   {
      client->SMTPName[ MAXADDR ] = '\0'; /* Truncate to allow ignoring
                                             length else where in code  */
   }

/*--------------------------------------------------------------------*/
/*            Format our name (and theirs) in the HELO reply.         */
/*--------------------------------------------------------------------*/

   sprintf( client->transmit.data,
            "%s Hello %.64s, pleased to meet you",
            E_domain,
            operands[0] );

   SMTPResponse( client, SR_OK_GENERIC, client->transmit.data );

   return KWTrue;
}

/*--------------------------------------------------------------------*/
/*       c o m m a n d N O O P                                        */
/*                                                                    */
/*       No-operation command                                         */
/*--------------------------------------------------------------------*/

commandNOOP(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{
   SMTPResponse( client, verb->successResponse, "OK" );
   return KWTrue;
}

/*--------------------------------------------------------------------*/
/*       c o m m a n d R S E T                                        */
/*                                                                    */
/*       Reset server state to allow new transaction                  */
/*--------------------------------------------------------------------*/

commandRSET(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{

   cleanupClientMail( client );
   SMTPResponse( client, verb->successResponse, "Reset state" );
   return KWTrue;
}

/*--------------------------------------------------------------------*/
/*       c o m m a n d Q U I T                                        */
/*                                                                    */
/*       Respond to server termination                                */
/*--------------------------------------------------------------------*/

commandQUIT(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{
   cleanupClientMail( client );
   sprintf( client->transmit.data,
            "%s Closing connection, adios",
            E_domain  );
   SMTPResponse( client, verb->successResponse, client->transmit.data );
   setClientClosed( client );
   return KWTrue;
}

/*--------------------------------------------------------------------*/
/*       c o m m a n d S e q u e n c e I g n o r e                    */
/*                                                                    */
/*       Respond to out of sequence command                           */
/*--------------------------------------------------------------------*/

commandSequenceIgnore(SMTPClient *client,
                      struct _SMTPVerb* verb,
                      char **operands )
{
   typedef struct _CMD_LKUP {
      SMTPMode mode;
      char *name;
      int  code;
   } CMD_LKUP;

   static CMD_LKUP table[] =
   {
      { SM_EXITING,     "QUIT",      503 },
      { SM_UNGREETED,   "HELO",      503 },
      { SM_IDLE,        "MAIL",      503 },
      { SM_ADDR_FIRST,  "RCPT",      503 },
      { SM_ADDR_SECOND, "DATA",      503 },
      { 0,              "different", 503 }
   };

   CMD_LKUP *current = table;

/*--------------------------------------------------------------------*/
/*       If the user skipped the HELO command but otherwise issued    */
/*       a valid command, issue a warning, reset our mode as if       */
/*       the HELO was received and reinvoke the command processor.    */
/*--------------------------------------------------------------------*/

   if (( getClientMode( client ) == SM_UNGREETED ) &&
       ( verb->validModes & SM_IDLE ))
   {
      sprintf( 0, "Client did not use HELO protocol.");
      setClientMode( client, SM_IDLE );
      SMTPInvokeCommand( client );  /* Run command in acceptable mode */
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*       Determine what command the user probably skipped, and use    */
/*       a good return code for it.                                   */
/*--------------------------------------------------------------------*/

   for ( ;; )
   {
      /* Accept entry if flag is empty or matches current mode  */

      if ( (! current->mode) ||
           ( current->mode & getClientMode( client )))
         break;

      current++;                    /* Examine next table entry */

   }

   sprintf( client->transmit.data,
            "Command %.4s issued out of sequence, "
            "expected %s command next",
            client->receive.data,
            current->name );

   SMTPResponse( client,
                 current->code,
                 client->transmit.data );

   return KWFalse;

} /* commandSequenceIgnore */

/*--------------------------------------------------------------------*/
/*       c o m m a n d E x i t i n g                                  */
/*                                                                    */
/*       Respond to remote that server is shutting down               */
/*--------------------------------------------------------------------*/

commandExiting(SMTPClient *client,
               struct _SMTPVerb* verb,
               char **operands )
{
   SMTPResponse( client,
                 SR_TE_SHUTDOWN,
                 "Server shutdown in progress, please try later" );
   cleanupClientMail( client );
   setClientClosed( client );
   return KWTrue;
}

/*--------------------------------------------------------------------*/
/*       c o m m a n d T e r m i n a t e d                            */
/*                                                                    */
/*       Handle a terminated (closed network connection) client       */
/*--------------------------------------------------------------------*/

commandTerminated(SMTPClient *client,
                  struct _SMTPVerb* verb,
                  char **operands )
{
   cleanupClientMail( client );
   setClientClosed( client );
   return KWTrue;
}

/*--------------------------------------------------------------------*/
/*       c o m m a n d S y n t a x                                    */
/*                                                                    */
/*       No-operation command                                         */
/*--------------------------------------------------------------------*/

commandSyntax(SMTPClient *client,
              struct _SMTPVerb* verb,
              char **operands )
{
   sprintf( client->transmit.data,
            "\"%.10s\" command is not understood (client state 0x%x)",
            client->receive.data,
            getClientMode( client ));
   SMTPResponse( client, verb->successResponse, client->transmit.data );
   return KWTrue;
}
