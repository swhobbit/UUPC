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
 *       $Id: smtplwc.c 1.1 1997/11/21 18:15:18 ahd Exp $
 *
 *       Revision History:
 *       $Log: smtplwc.c $
 *       Revision 1.1  1997/11/21 18:15:18  ahd
 *       Command processing stub SMTP daemon
 *
 */

/*--------------------------------------------------------------------*/
/*                           Include files                            */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include "smtplwc.h"
#include "smtprecv.h"
#include "smtpnetw.h"
#include "timestmp.h"

/*--------------------------------------------------------------------*/
/*                            Global files                            */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtplwc.c 1.1 1997/11/21 18:15:18 ahd Exp $");

currentfile();

/*--------------------------------------------------------------------*/
/*       c o m m a n d A c c e p t                                    */
/*                                                                    */
/*       Accept a new client from our master listening socket         */
/*--------------------------------------------------------------------*/

commandAccept(SMTPClient *master,
              struct _SMTPVerb* verb,
              char **operands )
{
   SMTPClient *client = initializeClient( getClientHandle( master ),
                                          KWTrue );

/*--------------------------------------------------------------------*/
/*         If the client initialized, insert it into the list         */
/*--------------------------------------------------------------------*/

   if ( client != NULL )
   {
      SMTPClient *current = master;

      /* Step to the last link of the list */
      while( current->next != NULL )
         current = current->next;

      current->next = client;

   } /* if ( client != NULL ) */

   return KWTrue;

} /* commandAccept */

/*--------------------------------------------------------------------*/
/*       c o m m a n d I n i t                                        */
/*                                                                    */
/*       Perform initial client command processing (greeting)         */
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

   if ( strlen( client->SMTPName ) >= MAXADDR )
   {
      client->SMTPName[ MAXADDR - 1 ] = '\0';
                                    /* Truncate to allow ignoring
                                       length else where in code     */
   }

   if ( equali( verb->name, "EHLO" ))
      client->esmtp = KWTrue;

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
   sprintf( client->transmit.data,
            "%s Closing connection, adios",
            E_domain  );
   SMTPResponse( client, verb->successResponse, client->transmit.data );
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
      { SM_EXITING,     "QUIT",      SR_PE_ORDERING },
      { SM_UNGREETED,   "HELO",      SR_PE_ORDERING },
      { SM_IDLE,        "MAIL",      SR_PE_ORDERING },
      { SM_ADDR_FIRST,  "RCPT",      SR_PE_ORDERING },
      { SM_ADDR_SECOND, "DATA",      SR_PE_ORDERING },
      { 0,              "different", SR_PE_ORDERING }
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
   return KWTrue;

} /* commandExiting */

/*--------------------------------------------------------------------*/
/*       c o m m a n d T i m e o u t                                  */
/*                                                                    */
/*       Drop a client which has been idle too long                   */
/*--------------------------------------------------------------------*/

KWBoolean
commandTimeout(SMTPClient *client,
               struct _SMTPVerb* verb,
               char **operands )
{
   SMTPResponse( client,
                 SR_TE_SHUTDOWN,
                 "Idle timeout, closing connection");
   return KWTrue;

} /* commandTimeout */

/*--------------------------------------------------------------------*/
/*       c o m m a n d T e r m i n a t e d                            */
/*                                                                    */
/*       Handle a terminated (closed network connection) client       */
/*--------------------------------------------------------------------*/

commandTerminated(SMTPClient *client,
                  struct _SMTPVerb* verb,
                  char **operands )
{
   return KWTrue;

}  /* commandTerminated */

/*--------------------------------------------------------------------*/
/*       c o m m a n d S y n t a x                                    */
/*                                                                    */
/*       Inform client we do not know the command issued              */
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

}  /* commandSyntax */
