#ifndef _POP3LWC_H
#define _POP3LWC_H

/*--------------------------------------------------------------------*/
/*       s m t p l w c . h                                            */
/*                                                                    */
/*       Light-weight POP3 server verb processors                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2000 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: pop3lwc.h 1.3 1999/01/04 03:56:09 ahd Exp $
 *
 *       Revision History:
 *       $Log: pop3lwc.h $
 *       Revision 1.3  1999/01/04 03:56:09  ahd
 *       Annual copyright change
 *
 *       Revision 1.2  1998/03/06 06:53:00  ahd
 *       Add dummy commands to make Netscape happy
 *
 *       Revision 1.1  1998/03/01 19:50:17  ahd
 *       Initial revision
 *
 */

#include "pop3clnt.h"
#include "smtpverb.h"

KWBoolean
commandInit(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandUSER(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandPASS(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandSequenceIgnore(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandXSENDER(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandAUTH(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

#endif /* _POP3LWC_H */
