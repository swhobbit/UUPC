#ifndef _POP3LWC_H
#define _POP3LWC_H

/*--------------------------------------------------------------------*/
/*       s m t p l w c . h                                            */
/*                                                                    */
/*       Light-weight POP3 server verb processors                     */
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
 *       $Id: pop3lwc.h 1.1 1998/03/01 19:50:17 ahd v1-12v $
 *
 *       Revision History:
 *       $Log: pop3lwc.h $
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
