/*--------------------------------------------------------------------*/
/*       p o p 3 u s e r . c                                          */
/*                                                                    */
/*       POP3 specific heavy processes                                */
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
#include "pop3user.h"
#include "pop3clnt.h"
#include "smtpnetw.h"
#include "timestmp.h"

/*--------------------------------------------------------------------*/
/*                            Global files                            */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtplwc.c 1.8 1998/03/01 01:32:32 ahd Exp $");

KWBoolean
commandLoadMailbox(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{
   SMTPResponse(client,
                PR_ERROR_GENERIC,
                "Command not implemented" );
   setClientMode( client, P3_AUTHORIZATION );
   return KWFalse;
}

KWBoolean
commandDataOutput(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{
   SMTPResponse(client,
                PR_ERROR_GENERIC,
                "Command not implemented" );
   return KWFalse;
}

KWBoolean
commandDELE(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{
   SMTPResponse(client,
                PR_ERROR_GENERIC,
                "Command not implemented" );
   return KWFalse;
}

KWBoolean
commandLIST(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{
   SMTPResponse(client,
                PR_ERROR_GENERIC,
                "Command not implemented" );
   return KWFalse;
}

KWBoolean
commandSTAT(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{
   SMTPResponse(client,
                PR_ERROR_GENERIC,
                "Command not implemented" );
   return KWFalse;
}

KWBoolean
commandTOP(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{
   SMTPResponse(client,
                PR_ERROR_GENERIC,
                "Command not implemented" );
   return KWFalse;
}

KWBoolean
commandUIDL(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{
   SMTPResponse(client,
                PR_ERROR_GENERIC,
                "Command not implemented" );
   return KWFalse;
}

/*--------------------------------------------------------------------*/
/*       c l e a n u p T r a n a c t i o n                            */
/*                                                                    */
/*       reset variables for a mail command                           */
/*--------------------------------------------------------------------*/

void
cleanupTransaction(SMTPClient *client)
{
   if (client->transaction == NULL)
      return;

} /* cleanupTransaction */
