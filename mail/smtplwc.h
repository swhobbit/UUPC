#ifndef _SMTPLWC_H
#define _SMTPLWC_H

/*--------------------------------------------------------------------*/
/*       s m t p l w c . h                                            */
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
 *       $Id: smtplwc.h 1.2 1997/11/24 02:53:26 ahd v1-12u $
 *
 *       Revision History:
 *       $Log: smtplwc.h $
 *       Revision 1.2  1997/11/24 02:53:26  ahd
 *       First working SMTP daemon which delivers mail
 *
 *       Revision 1.1  1997/11/21 18:16:32  ahd
 *       Command processing stub SMTP daemon
 *
 */

#include "smtpverb.h"

KWBoolean
commandAccept(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandInit(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandHELO(SMTPClient *client,
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
commandSequenceIgnore(SMTPClient *client,
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

#endif /* _SMTPLWC_H */
