#ifndef _SMTPSERV_H
#define _SMTPSERV_H
/*--------------------------------------------------------------------*/
/*       s m t p s e r v . h                                          */
/*                                                                    */
/*       SMTP server support routines for clients                     */
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
 *    $Id: smtpserv.h 1.4 1997/11/25 05:05:36 ahd v1-12u $
 *
 *    $Log: smtpserv.h $
 *    Revision 1.4  1997/11/25 05:05:36  ahd
 *    More robust SMTP daemon
 *
 *    Revision 1.3  1997/11/24 02:53:26  ahd
 *    First working SMTP daemon which delivers mail
 *
 *    Revision 1.2  1997/11/21 18:16:32  ahd
 *    Command processing stub SMTP daemon
 *
 *    Revision 1.1  1997/06/03 03:26:38  ahd
 *    Initial revision
 *
 */

#include "smtpclnt.h"

KWBoolean
flagReadyClientList( SMTPClient *master );

KWBoolean
processReadyClientList( SMTPClient *current );

void
timeoutClientList( SMTPClient *current );

void dropTerminatedClientList( SMTPClient *master );

void dropAllClientList( SMTPClient *master );

#endif  /* _SMTPSERV_H */
