/*--------------------------------------------------------------------*/
/*       s m t p c m m n . c                                          */
/*                                                                    */
/*       Common Light-weight SMTP server verb processors              */
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
 *       $Id: smtplwc.c 1.8 1998/03/01 01:32:32 ahd Exp $
 *
 *       Revision History:
 *       $Log: smtplwc.c $
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

RCSID("$Id: smtplwc.c 1.8 1998/03/01 01:32:32 ahd Exp $");

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
   sprintf(client->transmit.data,
            "\"%.10s\" command is not understood (client state 0x%x)",
            client->receive.data,
            getClientMode(client));
   SMTPResponse(client, verb->successResponse, client->transmit.data);
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
   sprintf(client->transmit.data,
            "%s Closing connection, adios",
            E_domain);
   SMTPResponse(client, verb->successResponse, client->transmit.data);
   return KWTrue;
}
