/*--------------------------------------------------------------------*/
/*       s m t p l w c . c                                            */
/*                                                                    */
/*       Light-weight SMTP server verb processors                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2000 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by the   */
/*       UUPC/extended license agreement.                             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: smtplwc.c 1.15 2000/05/12 12:35:45 ahd Exp ahd $
 *
 *       Revision History:
 *       $Log: smtplwc.c $
 *       Revision 1.15  2000/05/12 12:35:45  ahd
 *       Annual copyright update
 *
 *       Revision 1.14  1999/01/08 02:21:01  ahd
 *       Convert currentfile() to RCSID()
 *
 *       Revision 1.13  1999/01/04 03:54:27  ahd
 *       Annual copyright change
 *
 *       Revision 1.12  1998/11/01 20:38:15  ahd
 *       Correct "Not using HELO protocol" warning message to not crash system
 *
 * Revision 1.11  1998/04/24  03:30:13  ahd
 * Use local buffers, not client->transmit.buffer, for output
 * Rename receive buffer, use pointer into buffer rather than
 *      moving buffered data to front of buffer every line
 * Restructure main processing loop to give more priority
 *      to client processing data already buffered
 * Add flag bits to client structure
 * Add flag bits to verb tables
 *
 *       Revision 1.10  1998/03/03 03:53:54  ahd
 *       Routines to handle messages within a POP3 mailbox
 *
 *       Revision 1.9  1998/03/01 19:43:33  ahd
 *       First compiling POP3 server which accepts user id/password
 *
 *       Revision 1.8  1998/03/01 01:32:32  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.7  1997/11/29 13:03:13  ahd
 *       Clean up single client (hot handle) mode for OS/2, including correct
 *       network initialization, use unique client id (pid), and invoke all
 *       routines needed in main client loop.
 *
 *       Revision 1.6  1997/11/28 23:11:38  ahd
 *       Additional SMTP auditing, normalize formatting, more OS/2 SMTP fixes
 *
 *       Revision 1.5  1997/11/28 04:52:10  ahd
 *       Initial UUSMTPD OS/2 support
 *
 *       Revision 1.4  1997/11/26 03:34:11  ahd
 *       Correct SMTP timeouts, break out protocol from rest of daemon
 *
 *       Revision 1.3  1997/11/25 05:05:06  ahd
 *       More robust SMTP daemon
 *
 *       Revision 1.2  1997/11/24 02:52:26  ahd
 *       First working SMTP daemon which delivers mail
 *
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
#include "arpadate.h"

/*--------------------------------------------------------------------*/
/*                            Global files                            */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtplwc.c 1.15 2000/05/12 12:35:45 ahd Exp ahd $");

/*--------------------------------------------------------------------*/
/*       c o m m a n d I n i t                                        */
/*                                                                    */
/*       Perform initial client command processing (greeting)         */
/*--------------------------------------------------------------------*/

KWBoolean
commandInit(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{
   char xmitBuf[XMIT_LENGTH];

/*--------------------------------------------------------------------*/
/*       This message encourages E-SMTP processing (EHLO command),    */
/*       as while we don't support any special SMTP extensions, we    */
/*       can say that in response to the EHLO command.                */
/*--------------------------------------------------------------------*/

   sprintf(xmitBuf,
            "%s ESMTP (%s %s, built %s %s) on-line at %s",
            E_domain,
            compilep,
            compilev,
            compiled,
            compilet,
            arpadate());

   SMTPResponse(client, SR_OK_CONNECT, xmitBuf);

   return KWTrue;
}

/*--------------------------------------------------------------------*/
/*       c o m m a n d  H E L O                                       */
/*                                                                    */
/*       Respond to client greeting                                   */
/*--------------------------------------------------------------------*/

KWBoolean
commandHELO(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{
   char xmitBuf[XMIT_LENGTH];

/*--------------------------------------------------------------------*/
/*                 Save the name the client claims to be              */
/*                                                                    */
/*       We could check for a domain name (at least one period in     */
/*       the name), but we don't at this point.                       */
/*--------------------------------------------------------------------*/

   client->clientName = strdup(operands[0]);
   checkref(client->clientName);

   if (strlen(client->clientName) >= MAXADDR)
   {
      client->clientName[ MAXADDR - 1 ] = '\0';
                                    /* Truncate to allow ignoring
                                       length else where in code     */
   }

   if (equali(verb->name, "EHLO"))
      client->esmtp = KWTrue;

/*--------------------------------------------------------------------*/
/*            Format our name (and theirs) in the HELO reply.         */
/*--------------------------------------------------------------------*/

   sprintf(xmitBuf,
            "%s Hello %.64s (%s%s%s), pleased to meet you",
            E_domain,
            operands[0],
            client->connection.reverseLookup ?
                     client->connection.hostName : "",
            client->connection.reverseLookup ? " " : "",
            client->connection.hostAddr);

   SMTPResponse(client, SR_OK_GENERIC, xmitBuf);

   return KWTrue;
}

/*--------------------------------------------------------------------*/
/*       c o m m a n d R S E T                                        */
/*                                                                    */
/*       Reset server state to allow new transaction                  */
/*--------------------------------------------------------------------*/

KWBoolean
commandRSET(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{

   cleanupTransaction(client);
   SMTPResponse(client, verb->successResponse, "Reset state");
   return KWTrue;
}

/*--------------------------------------------------------------------*/
/*       c o m m a n d Q U I T                                        */
/*                                                                    */
/*       Respond to server termination                                */
/*--------------------------------------------------------------------*/

KWBoolean
commandQUIT(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{
   char xmitBuf[XMIT_LENGTH];
   sprintf(xmitBuf,
            "%s Closing connection, adios",
            E_domain);
   SMTPResponse(client, verb->successResponse, xmitBuf);
   return KWTrue;
}

/*--------------------------------------------------------------------*/
/*       c o m m a n d S e q u e n c e I g n o r e                    */
/*                                                                    */
/*       Respond to out of sequence command                           */
/*--------------------------------------------------------------------*/

KWBoolean
commandSequenceIgnore(SMTPClient *client,
                      struct _SMTPVerb* verb,
                      char **operands)
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
   char xmitBuf[XMIT_LENGTH];

/*--------------------------------------------------------------------*/
/*       If the user skipped the HELO command but otherwise issued    */
/*       a valid command, issue a warning, reset our mode as if       */
/*       the HELO was received and reinvoke the command processor.    */
/*--------------------------------------------------------------------*/

   if ((getClientMode(client) == SM_UNGREETED) &&
       (verb->validModes & SM_IDLE))
   {
      printmsg(0, "Client did not use HELO protocol.");
      setClientMode(client, SM_IDLE);
      SMTPInvokeCommand(client);    /* Run command in acceptable mode */
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*       Determine what command the user probably skipped, and use    */
/*       a good return code for it.                                   */
/*--------------------------------------------------------------------*/

   for (;;)
   {
      /* Accept entry if flag is empty or matches current mode  */

      if ((! current->mode) ||
           (current->mode & getClientMode(client)))
         break;

      current++;                    /* Examine next table entry */

   }

   sprintf(xmitBuf,
            "Command %.4s issued out of sequence, "
            "expected %s command next",
            client->receive.DataBuffer,
            current->name);

   SMTPResponse(client, current->code, xmitBuf);

   return KWFalse;

} /* commandSequenceIgnore */
