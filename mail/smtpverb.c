/*--------------------------------------------------------------------*/
/*       s m t p v e r b . c                                          */
/*                                                                    */
/*       SMTP verb parser for UUPC/extended                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: smtpverb.c 1.16 2001/03/12 13:56:08 ahd v1-13k $
 *
 *       Revision History:
 *       $Log: smtpverb.c $
 *       Revision 1.16  2001/03/12 13:56:08  ahd
 *       Annual Copyright update
 *
 *       Revision 1.15  2000/05/25 03:41:49  ahd
 *       Use more conservative buffering to avoid aborts
 *
 *       Revision 1.14  2000/05/12 12:35:45  ahd
 *       Annual copyright update
 *
 *       Revision 1.13  1999/01/08 02:21:05  ahd
 *       Convert currentfile() to RCSID()
 *
 *       Revision 1.12  1999/01/04 03:54:27  ahd
 *       Annual copyright change
 *
 *       Revision 1.11  1998/04/24 03:30:13  ahd
 *       Use local buffers, not client->transmit.buffer, for output
 *       Rename receive buffer, use pointer into buffer rather than
 *            moving buffered data to front of buffer every line
 *       Restructure main processing loop to give more priority
 *            to client processing data already buffered
 *       Add flag bits to client structure
 *       Add flag bits to verb tables
 *
 *       Revision 1.10  1998/03/03 03:54:42  ahd
 *       Revamp tokenizer to handle multiple tokens
 *
 *       Revision 1.9  1998/03/01 19:40:48  ahd
 *       First compiling POP3 server which accepts user id/password
 *
 *       Revision 1.8  1998/03/01 01:31:54  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.7  1997/11/30 04:21:39  ahd
 *       Always compare up to four characters of input to
 *       avoid accidently tripping on a leading period
 *
 *       Revision 1.6  1997/11/28 23:11:38  ahd
 *       Additional SMTP auditing, normalize formatting, more OS/2 SMTP fixes
 *
 *       Revision 1.5  1997/11/26 03:34:11  ahd
 *       Correct SMTP timeouts, break out protocol from rest of daemon
 *
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
#include "smtpcmds.h"
#include "smtpnetw.h"

/*--------------------------------------------------------------------*/
/*                      Global defines/variables                      */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtpverb.c 1.16 2001/03/12 13:56:08 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*       f r e e O p e r a n d s                                      */
/*                                                                    */
/*       Drop the operand list                                        */
/*--------------------------------------------------------------------*/

static void
freeOperands(char **operands)
{
   free(operands);

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
getOperands(SMTPClient *client, SMTPVerb *verb)
{
   static const char mName[] = "getOperands";

   char **list = NULL;
   char *token = client->receive.DataBuffer + strlen(verb->name);
   size_t subscript = 0;
   size_t maxEntries = client->receive.DataUsed / 2 + 1;

/*--------------------------------------------------------------------*/
/*         Perform limited case insensitive pattern matching          */
/*--------------------------------------------------------------------*/

   if (verb->pattern != NULL)
   {
      while(*token != '\0')
      {
         if (equalni(token, verb->pattern, strlen(verb->pattern)))
         {
            token += strlen(verb->pattern);
            break;
         }
         else
            token ++;

      } /* while(*token != '\0') */

   } /* if (verb->pattern != NULL) */

   list = malloc(maxEntries * sizeof *list);
   checkref(list);
   memset(list, 0, maxEntries * sizeof *list);

   /* Load the list of whitespace delimited list of tokens */
   if ((token != NULL) && (token != '\0'))
   {
      while((token = strtok(token, WHITESPACE)) != NULL)
      {
         list[subscript++] = token;
#ifdef UDEBUG
         printmsg(9, "%s: Token %d: \"%s\"", mName, subscript, token );
#endif
         token = NULL;
      }
   } /* if */

   /* Insure the list is terminated */
   list[subscript] = NULL;

   if ((verb->minOperands != SV_OPTIONAL_OPERANDS) &&
       (verb->minOperands > subscript))
   {
      SMTPResponse(client,
                   missingOperandError,
                   "Command operand is missing");
      return NULL;
   }

   printmsg(5, "%s Returning client %d total of %d tokens%s%s.",
               mName,
               getClientSequence(client),
               subscript,
               subscript ? " beginning with " : "",
               subscript ? list[0] : "");

   return list;

} /* getOperands */

/*--------------------------------------------------------------------*/
/*       S M T P I n v o k e C o m m a n d                            */
/*                                                                    */
/*       Select a command, parse operands if required, invoke         */
/*       the command, and step to next processing state if            */
/*       needed                                                       */
/*--------------------------------------------------------------------*/

void
SMTPInvokeCommand(SMTPClient *client)
{
   static const char mName[] = "SMTPInvokeCommand";
   SMTPVerb *currentVerb = verbTable;

   char **operands = NULL;

/*--------------------------------------------------------------------*/
/*               Locate the name of the verb to process               */
/*--------------------------------------------------------------------*/

   for (;;)
   {
      if ((getClientMode(client) & currentVerb->validModes) &&
            ! strlen(currentVerb->name))
         break;
      else if (*currentVerb->name == '\0')
      {
         /* No operation */
      }
#ifdef UDEBUG
      else if (client->receive.DataBuffer == NULL)
      {
         printmsg(0,"%s: Client %d Input line is NULL (client flags x%0x4)",
                     mName,
                     getClientSequence(client),
                     client->flag);
         panic();
      }
#endif
      else if (equalni(currentVerb->name,
                       client->receive.DataBuffer,
                       max(3, strlen(currentVerb->name))))
         break;

      currentVerb++;

   } /* for (;;) */

   if (currentVerb->flag & VF_TRIVIAL_CMD)
      incrementClientTrivialCount(client); /* Track for possible
                                              denial of service
                                              attack               */

/*--------------------------------------------------------------------*/
/*            If not proper mode for this verb, reject it             */
/*--------------------------------------------------------------------*/

   if (! (getClientMode(client) & currentVerb->validModes))
   {
      incrementClientTrivialCount(client);
      currentVerb->rejecter(client, currentVerb, NULL);
      return;
   }

/*--------------------------------------------------------------------*/
/*       If we can get the needed operands (if any), execute          */
/*       the command processor.  If operand parsing fails,            */
/*       it will issue the error message itself.                      */
/*--------------------------------------------------------------------*/

   if ((currentVerb->minOperands == 0) ||
       ((operands = getOperands(client, currentVerb)) != NULL))
   {
      if (currentVerb->processor(client, currentVerb, operands))
      {
         /* Special processing only for commands which succeed */

         /* If command worked, update client mode as needed */
         if (currentVerb->newMode != SM_SAME_MODE)
            setClientMode(client, currentVerb->newMode);
         /* If client has data in same mode, immediately process it */
         else if ((currentVerb->flag | VF_DATA_REDRIVE) &&
               (getClientBufferedData(client)))
               setClientProcess(client, KWTrue);

         /* If next command doesn't require read, set flag */
         if (currentVerb->flag & VF_NO_READ_SUCCESS)
            setClientFlag(client, SF_NO_READ);
      }

      if (operands != NULL)
         freeOperands(operands);

   } /* if */

   /* If next command doesn't require read, set flag */
   if (currentVerb->flag & VF_NO_READ)
      setClientFlag(client, SF_NO_READ);

   /* If next command doesn't require tokenize, set flag */
   if (currentVerb->flag & VF_NO_TOKENIZE)
      setClientFlag(client, SF_NO_TOKENIZE);

   /* Flag this client was active */
   time(&client->lastTransactionTime);

} /* SMTPInvokeCommand(SMTPClient *client) */
