#ifndef _POP3USER_H
#define _POP3USER_H

/*--------------------------------------------------------------------*/
/*       s m t p r e c v . h                                          */
/*                                                                    */
/*       SMTP commands which actually handle mail                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1998 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: pop3user.h 1.1 1998/03/01 19:45:01 ahd Exp $
 *
 *       Revision History:
 *       $Log: pop3user.h $
 *       Revision 1.1  1998/03/01 19:45:01  ahd
 *       Initial revision
 *
 *       Revision 1.2  1998/03/01 01:28:37  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.1  1997/11/21 18:16:32  ahd
 *       Command processing stub SMTP daemon
 *
 */

#include "pop3clnt.h"
#include "smtpverb.h"

KWBoolean
commandLoadMailbox(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandDataOutput(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandDELE(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandLIST(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandNOOP(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandQUIT(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandRETR(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandRSET(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands);

KWBoolean
commandSTAT(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandTOP(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandUIDL(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandUSER(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

#endif /* _POP3USER_H */
