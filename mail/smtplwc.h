#ifndef _SMTPLWC_H
#define _SMTPLWC_H

/*--------------------------------------------------------------------*/
/*       s m t p l w c . h                                            */
/*                                                                    */
/*       Light-weight SMTP server verb processors                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2002 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: smtplwc.h 1.8 2001/03/12 13:51:54 ahd v1-13k $
 *
 *       Revision History:
 *       $Log: smtplwc.h $
 *       Revision 1.8  2001/03/12 13:51:54  ahd
 *       Annual copyright update
 *
 *       Revision 1.7  2000/05/12 12:42:31  ahd
 *       Annual copyright update
 *
 *       Revision 1.6  1999/01/04 03:56:09  ahd
 *       Annual copyright change
 *
 *       Revision 1.5  1998/03/03 03:55:05  ahd
 *       Routines to handle messages within a POP3 mailbox
 *
 *       Revision 1.4  1998/03/01 19:43:52  ahd
 *       First compiling POP3 server which accepts user id/password
 *
 *       Revision 1.3  1998/03/01 01:28:37  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.2  1997/11/24 02:53:26  ahd
 *       First working SMTP daemon which delivers mail
 *
 *       Revision 1.1  1997/11/21 18:16:32  ahd
 *       Command processing stub SMTP daemon
 *
 */

#include "smtpverb.h"

KWBoolean
commandInit(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandHELO(SMTPClient *client,
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

#endif /* _SMTPLWC_H */
