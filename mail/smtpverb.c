/*--------------------------------------------------------------------*/
/*       s m t p v e r b . c                                          */
/*                                                                    */
/*       SMTP verb parser for UUPC/extended                           */
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
 *       $Id: smtpverb.c 1.2 1997/11/21 18:15:18 ahd Exp $
 *
 *       Revision History:
 *       $Log: smtpverb.c $
 *       Revision 1.2  1997/11/21 18:15:18  ahd
 *       Command processing stub SMTP daemon
 *
 *       Revision 1.1  1997/06/03 03:25:31  ahd
 *       Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                         Standard includes                          */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include "smtpverb.h"
#include "smtpnetw.h"

#include "smtplwc.h"
#include "smtprecv.h"

/*--------------------------------------------------------------------*/
/*                      Global defines/variables                      */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtpverb.c 1.2 1997/11/21 18:15:18 ahd Exp $");

currentfile();

/*--------------------------------------------------------------------*/
/*       Master command verb table                                    */
/*                                                                    */
/*       Empty verbs names are completely state driven and are        */
/*       skipped if client is not in proper state, all others are     */
/*       processed as an out of sequence command.                     */
/*--------------------------------------------------------------------*/

static SMTPVerb table[] =
{
   /* commandAccept only used by master socket to create clients */
   {
      commandAccept,
      commandSequenceIgnore,
      "",
      SM_MASTER,
      SM_SAME_MODE,
   },
   {
      commandInit,
      commandSequenceIgnore,
      "",
      SM_CONNECTED,
      SM_UNGREETED,

      SR_OK_CONNECT,
   },
   {
      commandExiting,
      commandSequenceIgnore,
      "",
      SM_EXITING,
      SM_INVALID,

      SR_TE_SHUTDOWN,
   },
   {
      commandTimeout,
      commandSequenceIgnore,
      "",
      SM_TIMEOUT,
      SM_INVALID,

      SR_TE_SHUTDOWN,
   },
   {
      commandTerminated,
      commandSequenceIgnore,
      "",
      SM_ABORT,
      SM_INVALID,
   },
   /* Period command has priority over SM_DATA mode */
   {
      commandPeriod,
      commandSequenceIgnore,
      ".",
      SM_DATA,
      SM_IDLE,

      SR_OK_MAIL_ACCEPTED,
      SR_PE_NEED_ADDR
   },
   {
      commandDataInput,
      commandSequenceIgnore,
      "",
      SM_DATA,
      SM_SAME_MODE,
   },
   {
      commandHELO,
      commandSequenceIgnore,
      "HELO",
      SM_UNGREETED,
      SM_IDLE,

      SR_OK_GENERIC,
      SR_PE_DUPLICATE,
      1
   },
   {
      commandHELO,
      commandSequenceIgnore,
      "EHLO",
      SM_UNGREETED,
      SM_IDLE,

      SR_OK_GENERIC,
      SR_PE_DUPLICATE,
      1
   },
   {
      commandRSET,
      commandSequenceIgnore,
      "RSET",
      SMTP_MODES_AFTER_HELO,
      SM_IDLE,

      SR_OK_GENERIC,
      SR_PE_DUPLICATE,
   },
   {
      commandNOOP,
      commandSequenceIgnore,
      "EXPN",
      SMTP_MODES_NONE,
      SM_SAME_MODE,

      SR_PE_NOT_POLICY
   },
   {
      commandVRFY,
      commandSequenceIgnore,
      "VRFY",
      SMTP_MODES_AFTER_HELO,
      SM_SAME_MODE,

      SR_OK_VRFY_LOCAL,
      SR_PE_ORDERING,
      1
   },
   {
      commandMAIL,
      commandSequenceIgnore,
      "MAIL",
      SM_IDLE,
      SM_ADDR_FIRST,

      SR_OK_SENDER,
      SR_PE_NOT_IDLE,
      1,
      "FROM:"
   },
   {
      commandRCPT,
      commandSequenceIgnore,
      "RCPT",
      ( SM_ADDR_FIRST | SM_ADDR_SECOND ),
      SM_ADDR_SECOND,

      SR_OK_RECEIPT,
      SR_PE_NOT_MAIL,
      1,
      "TO:"
   },
   {
      commandDATA,
      commandSequenceIgnore,
      "DATA",
      SM_ADDR_SECOND,
      SM_DATA,

      SR_OK_SEND_DATA,
   },
   {
      commandNOOP,
      commandSequenceIgnore,
      "NOOP",
      SMTP_MODES_AFTER_HELO,
      SM_SAME_MODE,

      SR_OK_GENERIC,
      0
   },
   {
      commandQUIT,
      commandSequenceIgnore,
      "QUIT",
      SMTP_MODES_ALL,
      SM_INVALID,

      SR_OK_QUIT,
      0
   },
   /* Command for syntax errors MUST BE LAST */
   {
      commandSyntax,             /* Fall through for syntax errs  */
      commandSequenceIgnore,
      "",
      SMTP_MODES_ALL,
      SM_SAME_MODE,

      SR_PE_UNKNOWN,
      0
   }
}; /* SMTPVerb table[] */

/*--------------------------------------------------------------------*/
/*       f r e e O p e r a n d s                                      */
/*                                                                    */
/*       Drop the operand list                                        */
/*--------------------------------------------------------------------*/

static void
freeOperands( char **operands )
{
   free( operands );

} /* freeOperands */

/*--------------------------------------------------------------------*/
/*       g e t O p e r a n d s                                        */
/*                                                                    */
/*       Parse arguments from the command line and put them           */
/*       into the argument list.                                      */
/*                                                                    */
/*       Currently supports only one operand, and points into         */
/*       original receive buffer for it                               */
/*--------------------------------------------------------------------*/

static char **
getOperands( SMTPClient *client, SMTPVerb *verb )
{
   static const char mName[] = "getOperands";

   char **list = NULL;
   char *token = client->receive.data + strlen( verb->name );

/*--------------------------------------------------------------------*/
/*         Perform limited case insensitive pattern matching          */
/*--------------------------------------------------------------------*/

   if ( verb->pattern != NULL )
   {
      while( *token != '\0' )
      {
         if ( equalni( token, verb->pattern, strlen(verb->pattern)))
         {
            token += strlen( verb->pattern );
            break;
         }
         else
            token ++;
      }

      if ( token == '\0' )
         token = NULL;

   } /* if ( verb->pattern != NULL ) */

   if ( token != NULL )
      token = strtok( token, WHITESPACE );

   if ( token == NULL )
   {
      SMTPResponse( client,
                    SR_PE_OPER_MISS,
                    "Command operand is missing" );
      return NULL;
   }

   list = malloc( 2 * sizeof *list );
   list[0] = token;

   printmsg(5, "%s Returning client %d token %s",
               mName,
               getClientSequence( client ),
               token );
   return list;
}

/*--------------------------------------------------------------------*/
/*       S M T P I n v o k e C o m m a n d                            */
/*                                                                    */
/*       Select a command, parse operands if required, invoke         */
/*       the command, and step to next processing state if            */
/*       needed                                                       */
/*--------------------------------------------------------------------*/

void
SMTPInvokeCommand( SMTPClient *client )
{
   SMTPVerb *currentVerb = table;

   char **operands = NULL;

/*--------------------------------------------------------------------*/
/*               Locate the name of the verb to process               */
/*--------------------------------------------------------------------*/

   for ( ;; )
   {
      if ( (getClientMode( client ) & currentVerb->validModes) &&
            ! strlen( currentVerb->name ))
         break;
      else if ( (*currentVerb->name != '\0') &&
                equalni( currentVerb->name,
                         client->receive.data,
                         sizeof currentVerb->name - 1 ))
         break;
      else
         currentVerb++;
   }

/*--------------------------------------------------------------------*/
/*            If not proper mode for this verb, reject it             */
/*--------------------------------------------------------------------*/

   if ( ! (getClientMode( client ) & currentVerb->validModes))
   {
      currentVerb->rejecter( client, currentVerb, NULL );
      return;
   }

/*--------------------------------------------------------------------*/
/*       If we can get the needed operands (if any), execute          */
/*       the command processor.  If operand parsing fails,            */
/*       it will issue the error message itself.                      */
/*--------------------------------------------------------------------*/

   if ((currentVerb->minOperands == 0 ) ||
       ((operands = getOperands( client, currentVerb )) != NULL ))
   {
      if ( currentVerb->processor( client, currentVerb, operands ) )
      {
         /* If command worked, update client mode as needed */
         if ( currentVerb->newMode != SM_SAME_MODE )
            setClientMode( client, currentVerb->newMode );
      }

      if ( operands != NULL )
         freeOperands( operands );

   } /* if */

} /* SMTPInvokeCommand( SMTPClient *client ) */
