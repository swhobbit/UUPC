#ifndef _SMTPSERV_H
#define _SMTPSERV_H
/*--------------------------------------------------------------------*/
/*       s m t p s e r v . h                                          */
/*                                                                    */
/*       SMTP server support routines for clients                     */
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
 *    $Id: smtpserv.h 1.2 1997/11/21 18:16:32 ahd Exp $
 *
 *    $Log: smtpserv.h $
 *    Revision 1.2  1997/11/21 18:16:32  ahd
 *    Command processing stub SMTP daemon
 *
 *    Revision 1.1  1997/06/03 03:26:38  ahd
 *    Initial revision
 *
 */

#include "smtpclnt.h"

KWBoolean
flagReadyClients( SMTPClient *master );

KWBoolean
processReadyClients( SMTPClient *current );

void
timeoutClients( SMTPClient *current );

void dropTerminatedClients( SMTPClient *master );

void dropAllClients( SMTPClient *master );

#endif  /* _SMTPSERV_H */
