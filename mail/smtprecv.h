#ifndef _SMTPRECV_H
#define _SMTPRECV_H

/*--------------------------------------------------------------------*/
/*       s m t p r e c v . h                                          */
/*                                                                    */
/*       SMTP commands which actually handle mail                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1999 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: smtprecv.h 1.2 1998/03/01 01:28:37 ahd v1-13f ahd $
 *
 *       Revision History:
 *       $Log: smtprecv.h $
 *       Revision 1.2  1998/03/01 01:28:37  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.1  1997/11/21 18:16:32  ahd
 *       Command processing stub SMTP daemon
 *
 */

#include "smtpverb.h"

KWBoolean
commandVRFY(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandMAIL(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandRCPT(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandDATA(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandDataInput(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

KWBoolean
commandPeriod(SMTPClient *client,
            struct _SMTPVerb* verb,
            char **operands );

#endif /* _SMTPRECV_H */
