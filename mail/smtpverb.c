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
 *       $Id: smtpverb.c 1.1 1997/06/03 03:25:31 ahd Exp $
 *
 *       Revision History:
 *       $Log: smtpverb.c $
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

RCSID("$Id: smtpverb.c 1.1 1997/06/03 03:25:31 ahd Exp $");

currentfile();

static SMTPVerb table[] = {
      {
         "",
         commandInit,
         commandSequenceIgnore,
         SM_CONNECTED,
         SM_UNGREETED,
         SR_OK_CONNECT,
         0,
      },
      {
         "",
         commandExiting,
         commandSequenceIgnore,
         SM_EXITING,
         SM_INVALID,
         SR_TE_SHUTDOWN,
         0,
      },
      {
         "",
         commandTerminated,
         commandSequenceIgnore,
         SM_ABORT,
         SM_INVALID,
         0,
         0,
      },
      {
         ".",
         commandPeriod,
         commandSequenceIgnore,
         SM_DATA,
         SM_PERIOD,
         SR_OK_SEND_DATA,
         SR_PE_NEED_ADDR
      },
      {
         "",
         commandDataInput,
         commandSequenceIgnore,
         SM_DATA,
         SM_SAME_MODE,
         SR_OK_SEND_DATA,
         0,
      },
      {
         "HELO",
         commandHELO,
         commandSequenceIgnore,
         SM_UNGREETED,
         SM_IDLE,
         SR_OK_GENERIC,
         SR_PE_DUPLICATE,
         1
      },
      {
         "EHLO",
         commandHELO,
         commandSequenceIgnore,
         SM_UNGREETED,
         SM_IDLE,
         SR_OK_GENERIC,
         SR_PE_DUPLICATE,
         1
      },
      {
         "RSET",
         commandRSET,
         commandSequenceIgnore,
         SMTP_MODES_AFTER_HELO,
         SM_IDLE,
         SR_OK_GENERIC,
         SR_PE_DUPLICATE,
         1
      },
      {
         "EXPN",
         commandNOOP,
         commandSequenceIgnore,
         SMTP_MODES_NONE,
         SM_SAME_MODE,
         SR_PE_NOT_POLICY
      },
      {
         "VRFY",
         commandVRFY,
         commandSequenceIgnore,
         SMTP_MODES_NONE,
         SMTP_MODES_AFTER_HELO,
         SR_PE_NOT_IMPL,
         1
      },
      {
         "MAIL",
         commandMAIL,
         commandSequenceIgnore,
         SM_IDLE,
         SM_ADDR_FIRST,
         SR_OK_SENDER,
         SR_PE_NOT_IDLE,
         1,
         "FROM:"
      },
      {
         "RCPT",
         commandRCPT,
         commandSequenceIgnore,
         ( SM_ADDR_FIRST | SM_ADDR_SECOND ),
         SM_ADDR_SECOND,
         SR_OK_RECEIPT,
         SR_PE_NOT_MAIL,
         1,
         "TO:"
      },
      {
         "DATA",
         commandDATA,
         commandSequenceIgnore,
         SM_ADDR_SECOND,
         SM_DATA,
         SR_PE_NOT_IMPL
      },
      {
         "NOOP",
         commandNOOP,
         commandSequenceIgnore,
         SMTP_MODES_AFTER_HELO,
         SM_SAME_MODE,
         SR_OK_GENERIC,
         0
      },
      {
         "QUIT",
         commandQUIT,
         commandSequenceIgnore,
         SMTP_MODES_ALL,
         SM_INVALID,
         SR_OK_QUIT,
         0
      },
      {
         "",
         commandSyntax,             /* Fall through for syntax errs  */
         commandSequenceIgnore,
         SMTP_MODES_ALL,
         SM_SAME_MODE,
         SR_PE_UNKNOWN,
         0
      }
   };

static void
freeOperands( char **operands )
{
   free( operands );

} /* freeOperands */

static char **
getOperands( SMTPClient *client, SMTPVerb *verb )
{

   char **list = NULL;
   char *token = client->receive.data + strlen( verb->name );

   if ( verb->pattern != NULL )
   {
      /* This needs to be case insensitive */
      token = strstr( token, verb->pattern );
   }

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

   return list;
}

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
