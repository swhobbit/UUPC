/*--------------------------------------------------------------------*/
/*       s m t p r e c v . c                                          */
/*                                                                    */
/*       SMTP commands which actually handle mail                     */
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
 *       $Log$
 */

#include "uupcmoah.h"
#include "smtprecv.h"
#include "smtpnetw.h"

RCSID("$Id: smtpclnt.c 1.1 1997/06/03 03:25:31 ahd Exp $");

KWBoolean
commandVRFY(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{
   SMTPResponse( client, SR_OK_VRFY_REMOTE, "Who is to say?");
   return KWTrue;
}

KWBoolean
commandMAIL(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{
   SMTPResponse( client, SR_OK_SENDER, "Okay, send receiver addresses");
   return KWTrue;
}

KWBoolean
commandRCPT(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{
   SMTPResponse( client, SR_OK_RECEIPT, "I should know how to do that, but don't!");
   return KWTrue;
}

KWBoolean
commandDATA(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{
   SMTPResponse( client, SR_OK_SEND_DATA, "I should know how to do that, but don't!");
   return KWTrue;
}

KWBoolean
commandDataInput(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{

   printmsg(2,"Got data line.");
   return KWTrue;
}

KWBoolean
commandPeriod(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands )
{
   SMTPResponse( client, SR_OK_END_DATA, "I should know how to do that, but don't!");
   return KWTrue;
}
