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
 *       $Id: smtpverb.c 1.4 1997/11/25 05:05:06 ahd Exp $
 *
 *       Revision History:
 *       $Log: smtpverb.c $
 *       Revision 1.4  1997/11/25 05:05:06  ahd
 *       More robust SMTP daemon
 *
 *       Revision 1.3  1997/11/24 02:52:26  ahd
 *       First working SMTP daemon which delivers mail
 *
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

/*--------------------------------------------------------------------*/
/*                      Global defines/variables                      */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtpverb.c 1.4 1997/11/25 05:05:06 ahd Exp $");

currentfile();

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
   SMTPVerb *currentVerb = verbTable;

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
      incrementClientTrivialCount( client );
      currentVerb->rejecter( client, currentVerb, NULL );
      return;
   }

   if ( currentVerb->trivial )
      incrementClientTrivialCount( client ); /* Track for possible
                                                denial of service
                                                attack               */

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

   /* Flag this client was active */
   time( &client->lastTransactionTime );

} /* SMTPInvokeCommand( SMTPClient *client ) */
