#ifndef _SMTPCMMN_H
#define _SMTPCMMN_H

/*--------------------------------------------------------------------*/
/*       s m t p c m m n . h                                          */
/*                                                                    */
/*       Light-weight SMTP server verb processors                     */
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
 *       $Id: smtplwc.h 1.3 1998/03/01 01:28:37 ahd Exp $
 *
 *       Revision History:
 *       $Log: smtplwc.h $
 *
 */

#include "smtpverb.h"

KWBoolean
commandAccept(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandNOOP(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandRSET(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandQUIT(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandExiting(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandTerminated(SMTPClient *client,
                  struct _SMTPVerb* verb,
                  char **operands );

KWBoolean
commandTimeout(SMTPClient *client,
               struct _SMTPVerb* verb,
               char **operands );

KWBoolean
commandSyntax(SMTPClient *client,
              struct _SMTPVerb* verb,
              char **operands );

#endif /* _SMTPCMMN_H */
