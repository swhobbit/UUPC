#ifndef _SMTPCMMN_H
#define _SMTPCMMN_H

/*--------------------------------------------------------------------*/
/*       s m t p c m m n . h                                          */
/*                                                                    */
/*       Light-weight SMTP server verb processors                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2001 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: smtpcmmn.h 1.4 2000/05/12 12:42:31 ahd v1-13g $
 *
 *       Revision History:
 *       $Log: smtpcmmn.h $
 *       Revision 1.4  2000/05/12 12:42:31  ahd
 *       Annual copyright update
 *
 *       Revision 1.3  1999/01/04 03:56:09  ahd
 *       Annual copyright change
 *
 *       Revision 1.2  1998/03/03 03:55:05  ahd
 *       Routines to handle messages within a POP3 mailbox
 *
 *       Revision 1.1  1998/03/01 19:45:01  ahd
 *       Initial revision
 *
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
