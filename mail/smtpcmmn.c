/*--------------------------------------------------------------------*/
/*       s m t p c m m n . c                                          */
/*                                                                    */
/*       Common Light-weight SMTP server verb processors              */
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
 *       $Id: smtpcmmn.c 1.4 1998/04/24 03:30:13 ahd v1-13f ahd $
 *
 *       Revision History:
 *       $Log: smtpcmmn.c $
 *       Revision 1.4  1998/04/24 03:30:13  ahd
 *       Use local buffers, not client->transmit.buffer, for output
 *       Rename receive buffer, use pointer into buffer rather than
 *            moving buffered data to front of buffer every line
 *       Restructure main processing loop to give more priority
 *            to client processing data already buffered
 *       Add flag bits to client structure
 *       Add flag bits to verb tables
 *
 *       Revision 1.3  1998/03/08 23:10:20  ahd
 *       Don't try to write to master (listening) socket
 *
 *       Revision 1.2  1998/03/03 03:53:54  ahd
 *       Remove RSET and QUIT commands, which really have to be unique to SMTP and POP3
 *
 *       Revision 1.1  1998/03/01 19:42:17  ahd
 *       Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                           Include files                            */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include "smtpcmmn.h"
#include "smtprecv.h"
#include "smtpnetw.h"

/*--------------------------------------------------------------------*/
/*                            Global files                            */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtpcmmn.c 1.4 1998/04/24 03:30:13 ahd v1-13f ahd $");

/*--------------------------------------------------------------------*/
/*       c o m m a n d A c c e p t                                    */
/*                                                                    */
/*       Accept a new client from our master listening socket         */
/*--------------------------------------------------------------------*/

KWBoolean
commandAccept(SMTPClient *master,
              struct _SMTPVerb* verb,
              char **operands)
{
   SMTPClient *client;

   setClientReady(master, KWFalse);

/*--------------------------------------------------------------------*/
/*         If the client initialized, insert it into the list         */
/*--------------------------------------------------------------------*/

   client = initializeClient(getClientHandle(master), KWTrue);

   if (client != NULL)
   {
      SMTPClient *current = master;

      /* Step to the last link of the list */
      while(current->next != NULL)
         current = current->next;

      client->previous = current;
      current->next = client;

   } /* if (client != NULL) */

   incrementClientMajorTransaction(master);
   return KWTrue;

} /* commandAccept */

/*--------------------------------------------------------------------*/
/*       c o m m a n d E x i t i n g                                  */
/*                                                                    */
/*       Respond to remote that server is shutting down               */
/*--------------------------------------------------------------------*/

KWBoolean
commandExiting(SMTPClient *client,
               struct _SMTPVerb* verb,
               char **operands)
{
   if ( ! client->listening )
      SMTPResponse(client,
                   verb->successResponse,
                  "Server shutdown in progress, please try later");
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
               char **operands)
{
   if ( ! client->listening )
      SMTPResponse(client,
                    verb->successResponse,
                    "Idle timeout, closing connection");

   return KWTrue;

} /* commandTimeout */

/*--------------------------------------------------------------------*/
/*       c o m m a n d T e r m i n a t e d                            */
/*                                                                    */
/*       Handle a terminated (closed network connection) client       */
/*--------------------------------------------------------------------*/

KWBoolean
commandTerminated(SMTPClient *client,
                  struct _SMTPVerb* verb,
                  char **operands)
{
   return KWTrue;

}  /* commandTerminated */

/*--------------------------------------------------------------------*/
/*       c o m m a n d S y n t a x                                    */
/*                                                                    */
/*       Inform client we do not know the command issued              */
/*--------------------------------------------------------------------*/

KWBoolean
commandSyntax(SMTPClient *client,
              struct _SMTPVerb* verb,
              char **operands)
{
   char xmitBuf[XMIT_LENGTH];
   char *command = strtok(client->receive.line, " ");

   if (command == NULL)
      command = "(null)";

   sprintf(xmitBuf,
            "\"%.10s\" command is not understood (client state 0x%x)",
            command,
            getClientMode(client));
   SMTPResponse(client, verb->successResponse, xmitBuf);
   return KWTrue;

}  /* commandSyntax */

/*--------------------------------------------------------------------*/
/*       c o m m a n d N O O P                                        */
/*                                                                    */
/*       No-operation command                                         */
/*--------------------------------------------------------------------*/

KWBoolean
commandNOOP(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands)
{
   SMTPResponse(client, verb->successResponse, "OK (No operation)");
   return KWTrue;
}
